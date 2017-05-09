#include <Eigen/Dense>

#include <iostream>
#include <numeric>
#include <vector>

#include "glm/glm.hpp"
#include "path.hpp"

namespace tomovis {

template <typename T>
std::vector<Eigen::DenseIndex>
    sorted_merge_indices(Eigen::Matrix<T, Eigen::Dynamic, 1> const& v1,
                         Eigen::Matrix<T, Eigen::Dynamic, 1> const& v2) {
    std::vector<Eigen::DenseIndex> idcs;
    Eigen::DenseIndex i1 = 0, i2 = 0, ins_idx = 0;
    while (true) {
        if (i2 == v2.size()) {
            break;
        }

        if (i1 == v1.size()) {
            idcs.push_back(ins_idx);
            if (ins_idx < v1.size()) {
                ++ins_idx;
            }
            ++i2;
        } else {
            if (v1(i1) < v2(i2)) {
                ++ins_idx;
                ++i1;
            } else {
                idcs.push_back(ins_idx);
                ++i2;
            }
        }
    }
    return idcs;
}

std::ostream& operator<<(std::ostream& out, bdry_cond bc) {
    switch (bc) {
    case bdry_cond::natural:
        out << "n";
        break;
    case bdry_cond::clamp:
        out << "c";
        break;
    case bdry_cond::zero:
        out << "0";
        break;
    default:
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, BdryConds3 const& bcs) {
    std::cout << "boundary conditions" << std::endl;
    for (auto bc : bcs.bdry_conds()) {
        std::cout << bc.first << " " << bc.second << std::endl;
    }
    return out;
}

Eigen::Matrix<float, Eigen::Dynamic, 3>
vec_of_vecs_to_matrix(std::vector<glm::vec3> const& vec_of_vecs) {
    Eigen::Matrix<float, Eigen::Dynamic, 3> matrix(vec_of_vecs.size(), 3);
    for (size_t i = 0; i < vec_of_vecs.size(); ++i) {
        matrix.row(i) << vec_of_vecs[i][0], vec_of_vecs[i][1],
            vec_of_vecs[i][2];
    }
    return matrix;
}

Path3::Path3(Eigen::Matrix<float, Eigen::Dynamic, 3> const& nodes, bdry_cond bc)
    : Path3(nodes, BdryConds3(bc)) {}

Path3::Path3(std::vector<glm::vec3> const& nodes, bdry_cond bc)
    : Path3(vec_of_vecs_to_matrix(nodes), bc) {}

Path3::Path3(Eigen::Matrix<float, Eigen::Dynamic, 3> const& nodes,
             BdryConds3 const& bcs)
    : nodes_(nodes), bdry_conds_(bcs) {
    assert(nodes.rows() >= 2);
    compute_matrices_();
    init_tangents_();
    init_rhs_();
    compute_tangents_();
    init_a_and_b_vecs_();
}

Path3::Path3(std::vector<glm::vec3> const& nodes, BdryConds3 const& bcs)
    : Path3(vec_of_vecs_to_matrix(nodes), bcs) {}

Path3::Path3(Eigen::Matrix<float, Eigen::Dynamic, 3> const& nodes,
             Eigen::RowVector3f tang_left, Eigen::RowVector3f tang_right,
             bdry_cond bc)
    : Path3(nodes, tang_left, tang_right, BdryConds3(bc)) {}

Path3::Path3(std::vector<glm::vec3> const& nodes, glm::vec3 tang_left,
             glm::vec3 tang_right, bdry_cond bc)
    : Path3(vec_of_vecs_to_matrix(nodes),
            {tang_left[0], tang_left[1], tang_left[2]},
            {tang_right[0], tang_right[1], tang_right[2]}, bc) {}

Path3::Path3(Eigen::Matrix<float, Eigen::Dynamic, 3> const& nodes,
             Eigen::RowVector3f tang_left, Eigen::RowVector3f tang_right,
             BdryConds3 const& bcs)
    : nodes_(nodes), bdry_conds_(bcs) {
    assert(nodes.rows() >= 2);
    compute_matrices_();
    init_tangents_(tang_left, tang_right);
    init_rhs_();
    compute_tangents_();
    init_a_and_b_vecs_();
}

Path3::Path3(std::vector<glm::vec3> const& nodes, Eigen::RowVector3f tang_left,
             Eigen::RowVector3f tang_right, BdryConds3 const& bcs)
    : Path3(vec_of_vecs_to_matrix(nodes), tang_left, tang_right, bcs) {}

// Operator overloads

Eigen::RowVector3f Path3::operator()(float param) const {
    assert((param >= 0) && (param <= num_pieces()));

    auto piece = static_cast<Eigen::DenseIndex>(floor(param));
    float s = param - piece; // Normalized to [0, 1]
    if (piece == num_pieces()) {
        return nodes().row(num_pieces());
    } // right end of the parameter range

    // Compute path points as
    //     path(s) = p[k] + s(p[k+1] - p[k]) + s(1-s)((1-s) a[k] + s b[k]),
    // where p = nodes, s = param in [0, 1], and a, b as below.
    // See https://en.wikipedia.org/wiki/Spline_interpolation
    return nodes().row(piece) +
           s * (nodes().row(piece + 1) - nodes().row(piece)) +
           s * (1.0 - s) *
               ((1.0 - s) * a_vecs_.row(piece) + s * b_vecs_.row(piece));
}

Eigen::Matrix<float, Eigen::Dynamic, 3> Path3::
operator()(Eigen::VectorXf const& params) const {
    Eigen::Matrix<float, Eigen::Dynamic, 3> points(params.size(), 3);
    for (Eigen::DenseIndex i = 0; i < params.size(); ++i) {
        points.row(i) = this->operator()(params(i));
    }
    return points;
}

std::ostream& operator<<(std::ostream& out, Path3 const& p) {
    out << "Path3, using nodes" << std::endl
        << p.nodes() << std::endl
        << "and " << p.bdry_conds();
    return out;
}

// Utilities for constructing the system of equations for the tangents

Eigen::MatrixXf Path3::system_matrix(bdry_cond bc_left,
                                     bdry_cond bc_right) const {
    // Return the matrix of the equation system for the tangents of the path
    // at
    // the given nodes.
    Eigen::DenseIndex n_nodes = num_nodes();
    Eigen::MatrixXf sys_mat = Eigen::MatrixXf::Zero(n_nodes, n_nodes);

    // All rows except the first and last have an entry 4.0 on the diagonal
    // and
    // 1.0 on the first off-diagonals,
    // elsewhere 0.0.
    for (Eigen::DenseIndex row = 1; row < n_nodes - 1; ++row) {
        sys_mat(row, row) = 4.0;
        sys_mat(row, row - 1) = 1.0;
        sys_mat(row, row + 1) = 1.0;
    }

    switch (bc_left) {
    case bdry_cond::zero:
    case bdry_cond::clamp:
        // Just an identity equation, keeps original left tangent
        sys_mat(0, 0) = 1.0;
        break;
    case bdry_cond::natural:
        sys_mat(0, 0) = 2.0;
        sys_mat(0, 1) = 1.0;
        break;
    default:
        std::cout << "Invalid left boundary condition " << bc_left << "!";
    }
    switch (bc_right) {
    case bdry_cond::zero:
    case bdry_cond::clamp:
        // Just an identity equation, keeps original right tangent
        sys_mat(n_nodes - 1, n_nodes - 1) = 1.0;
        break;
    case bdry_cond::natural:
        sys_mat(n_nodes - 1, n_nodes - 1) = 2.0;
        sys_mat(n_nodes - 1, n_nodes - 2) = 1.0;
        break;
    default:
        std::cout << "Invalid right boundary condition " << bc_right << "!";
    }
    return sys_mat;
}
Eigen::VectorXf Path3::system_rhs(bdry_cond bc_left, bdry_cond bc_right,
                                  int dim) const {
    // Return the right-hand side of the equation system for the tangents of
    // the
    // path at the given nodes.
    Eigen::Matrix<float, Eigen::Dynamic, 3> nds = nodes();
    Eigen::DenseIndex n_nodes = nds.rows();
    Eigen::VectorXf sys_rhs(n_nodes);

    // All equations except the first and last have 3.0 times the difference
    // between the next and previous nodes
    // as right-hand sides.
    for (Eigen::DenseIndex i = 1; i < n_nodes - 1; ++i) {
        sys_rhs(i) = 3.0 * (nds(i + 1, dim) - nds(i - 1, dim));
    }

    switch (bc_left) {
    case bdry_cond::zero:
        sys_rhs(0) = 0.0;
        break;
    case bdry_cond::clamp:
        sys_rhs(0) = tangents()(0, dim);
        break;
    case bdry_cond::natural:
        sys_rhs(0) = 3.0 * (nds(1, dim) - nds(0, dim));
        break;
    default:
        break;
    }
    switch (bc_right) {
    case bdry_cond::zero:
        sys_rhs(n_nodes - 1) = 0.0;
        break;
    case bdry_cond::clamp:
        sys_rhs(n_nodes - 1) = nds(n_nodes - 1, dim);
        break;
    case bdry_cond::natural:
        sys_rhs(n_nodes - 1) =
            3.0 * (nds(n_nodes - 1, dim) - nds(n_nodes - 2, dim));
        break;
    default:
        break;
    }
    return sys_rhs;
}

// Internal helpers

void Path3::compute_matrices_() {
    for (int dim = 0; dim < 3; ++dim) {
        Eigen::MatrixXf sys_mat =
            system_matrix(bdry_conds_[dim].first, bdry_conds_[dim].second);
        Eigen::PartialPivLU<Eigen::MatrixXf> decomp = sys_mat.partialPivLu();
        sys_matrix_decomps_.push_back(decomp);
    }
}

void Path3::init_tangents_() {
    const Eigen::RowVector3f tang_left = nodes_.row(1) - nodes_.row(0);
    const Eigen::RowVector3f tang_right =
        nodes_.row(num_nodes() - 1) - nodes_.row(num_nodes() - 2);
    init_tangents_(tang_left, tang_right);
}

void Path3::init_tangents_(Eigen::RowVector3f const& tang_left,
                           Eigen::RowVector3f const& tang_right) {
    tangents_.resize(num_nodes(), 3);
    tangents_.row(0) = tang_left;
    tangents_.row(num_nodes() - 1) = tang_right;
}

void Path3::init_rhs_() {
    for (size_t dim = 0; dim < 3; ++dim) {
        sys_rhs_.push_back(this->system_rhs(bdry_conds_[dim].first,
                                            bdry_conds_[dim].second, dim));
    }
}

void Path3::compute_tangents_() {
    for (int dim = 0; dim < 3; ++dim) {
        tangents_.col(dim) =
            this->sys_matrix_decomps_[dim].solve(this->sys_rhs_[dim]);
    }
}

void Path3::init_a_and_b_vecs_() {
    a_vecs_.resize(num_pieces(), 3);
    b_vecs_.resize(num_pieces(), 3);
    for (Eigen::DenseIndex i = 0; i < num_pieces(); ++i) {
        a_vecs_.row(i) = tangents_.row(i) - (nodes_.row(i + 1) - nodes_.row(i));
        b_vecs_.row(i) =
            (nodes_.row(i + 1) - nodes_.row(i)) - tangents_.row(i + 1);
    }
}

// Arc length stuff

Eigen::VectorXf Path3::arc_length_lin_approx(size_t num_params) const {
    // Compute an approximation to the arc lengths at `num_params` equidistant
    // parameters using a piecewise constant path instead of the actual spline.
    // This is done by computing the path points at the parameters and then
    // accumulating the lengths of the differences of the points.
    Eigen::VectorXf params =
        Eigen::VectorXf::LinSpaced(num_params, 0, num_pieces());
    Eigen::Matrix<float, Eigen::Dynamic, 3> path_pts = this->operator()(params);
    Eigen::VectorXf alens(params.size());
    alens(0) = 0.0;
    for (Eigen::DenseIndex i = 1; i < params.size(); ++i) {
        alens(i) = (path_pts.row(i) - path_pts.row(i - 1)).norm();
    }
    float acc = 0.0;
    for (Eigen::DenseIndex i = 0; i < params.size(); ++i) {
        acc += alens(i);
        alens(i) = acc;
    }
    return alens;
}

Eigen::VectorXf Path3::arc_length_params_lin_approx(size_t num_params) const {
    // Interpolate the parameters that yield points that are equally spaced with
    // respect to arc length.
    // This is the discrete counterpart of reparametrization with respect to arc
    // length.
    //
    // See
    // https://en.wikipedia.org/wiki/Differential_geometry_of_curves#Length_and_natural_parametrization
    // for details.
    Eigen::VectorXf alen_params(num_params);
    Eigen::VectorXf params =
        Eigen::VectorXf::LinSpaced(num_params, 0.0f, num_pieces());
    Eigen::VectorXf alens = arc_length_lin_approx(num_params);
    Eigen::VectorXf target_alens =
        Eigen::VectorXf::LinSpaced(num_params, 0.0f, total_length(num_params));
    std::vector<Eigen::DenseIndex> pieces =
        sorted_merge_indices(alens, target_alens);
    for (size_t i = 0; i < num_params; ++i) {
        // Interpolate the target arc length in the computed arc lengths.
        float s; // Normalized distance to target node to the left
        if (pieces[i] == 0) {
            s = 0.0f;
            pieces[i] = 1;
        } else if (static_cast<size_t>(pieces[i]) == num_params) {
            s = 1.0f;
            pieces[i] = num_params - 1;
        } else {
            s = (target_alens(i) - alens(pieces[i] - 1)) /
                (alens(pieces[i]) - alens(pieces[i] - 1));
        }
        alen_params(i) =
            (1.0f - s) * params(pieces[i] - 1) + s * params(pieces[i]);
    }
    return alen_params;
}

float Path3::total_length(size_t num_params) const {
    Eigen::VectorXf alens = arc_length_lin_approx(num_params);
    return alens(alens.size() - 1);
}

// Derivatives and derived vectors

Eigen::RowVector3f Path3::deriv1(float param) const {
    assert(0 <= param && param <= num_pieces());
    auto piece = static_cast<Eigen::DenseIndex>(floor(param));
    if (piece == num_pieces()) {
        return tangents().row(num_pieces());
    } // right end of the parameter range
    auto s = param - piece;
    // Formula for the first derivative is
    // path'(s) = p[k+1] - p[k] + (1-s)(1-3s) a[k] + s(2-3s) b[k]
    return nodes().row(piece + 1) - nodes().row(piece) +
           (1.0f - s) * (1.0f - 3.0f * s) * a_vecs_.row(piece) +
           s * (2.0f - 3.0f * s) * b_vecs_.row(piece);
}

Eigen::RowVector3f Path3::deriv2(float param) const {
    assert(0 <= param && param <= num_pieces());
    auto piece = static_cast<Eigen::DenseIndex>(floor(param));
    if (piece == num_pieces()) {
        return 2.0f * a_vecs_.row(num_pieces() - 1) -
               4.0f * b_vecs_.row(num_pieces() - 1);
    } // right end of the parameter range
    auto s = param - piece;
    // Formula for the second derivative is
    // path''(s) = (-4+6s) a[k] + (2-6s) b[k]
    return (-4.0f + 6.0f * s) * a_vecs_.row(piece) +
           (2.0f - 6.0f * s) * b_vecs_.row(piece);
}

Eigen::RowVector3f Path3::deriv3(float param) const {
    assert(0 <= param && param <= num_pieces());
    auto piece = static_cast<Eigen::DenseIndex>(floor(param));
    if (piece == num_pieces()) {
        piece = num_pieces() - 1;
    } // right end of the parameter range
    // Formula for the second derivative is
    // path''(s) = 6 (a[k] - b[k])
    return 6.0f * (a_vecs_.row(piece) - b_vecs_.row(piece));
}

Eigen::RowVector3f Path3::unit_tangent(float param) const {
    auto d1 = deriv1(param);
    auto d1_norm = d1.norm();
    if (d1_norm < 1e-6) {
        return Eigen::RowVector3f::Zero();
    }
    return d1 / d1_norm;
}

Eigen::RowVector3f Path3::unit_normal(float param) const {
    auto unit_tan = unit_tangent(param);
    auto d2 = deriv2(param);
    auto normal = d2 - d2.dot(unit_tan) * unit_tan;
    auto normal_norm = normal.norm();
    if (normal_norm < 1e-6) {
        return Eigen::RowVector3f::Zero();
    }
    return normal / normal_norm;
}

Eigen::RowVector3f Path3::unit_binormal(float param) const {
    return unit_tangent(param).cross(unit_normal(param));
}

} // namespace tomovis
