#pragma once

#include <iostream>
#include <utility>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <glm/glm.hpp>

namespace tomovis {

enum class bdry_cond : int {
    zero,
    natural,
    clamp,
};

std::ostream& operator<<(std::ostream& out, bdry_cond bc);

class BdryConds3 {
  public:
    BdryConds3(bdry_cond bc = bdry_cond::natural) : BdryConds3(bc, bc) {}
    BdryConds3(bdry_cond bc_left, bdry_cond bc_right) {
        for (size_t dim = 0; dim < 3; dim++) {
            bdry_conds_.push_back(
                std::pair<bdry_cond, bdry_cond>(bc_left, bc_right));
        }
    }
    BdryConds3(bdry_cond bc0, bdry_cond bc1, bdry_cond bc2) {
        bdry_conds_.push_back(std::pair<bdry_cond, bdry_cond>(bc0, bc0));
        bdry_conds_.push_back(std::pair<bdry_cond, bdry_cond>(bc1, bc1));
        bdry_conds_.push_back(std::pair<bdry_cond, bdry_cond>(bc2, bc2));
    }
    BdryConds3(std::vector<std::pair<bdry_cond, bdry_cond>> const& bcs)
        : bdry_conds_(bcs) {}

    std::vector<std::pair<bdry_cond, bdry_cond>> bdry_conds() const {
        return bdry_conds_;
    }

    std::pair<bdry_cond, bdry_cond> operator[](size_t index) const {
        return bdry_conds_[index];
    }

  private:
    std::vector<std::pair<bdry_cond, bdry_cond>> bdry_conds_;
}; // class BdryConds3

std::ostream& operator<<(std::ostream& out, BdryConds3 const& bcs);

class Path3 {
  public:
    // Constructors
    // There are effectively 4 variants, each of which works with an
    // Eigen::Matrix or a std::vector of glm::vec3.

    // Same boundary condition left & right and in all dimensions
    Path3(Eigen::Matrix<float, Eigen::Dynamic, 3> const& nodes,
          bdry_cond bc = bdry_cond::natural);
    Path3(std::vector<glm::vec3> const& nodes,
          bdry_cond bc = bdry_cond::natural);

    // Custom boundary condition object, full flexibility
    Path3(Eigen::Matrix<float, Eigen::Dynamic, 3> const& nodes,
          BdryConds3 const& bcs);
    Path3(std::vector<glm::vec3> const& nodes, BdryConds3 const& bcs);

    // Set tangents at left and right endpoints explicitly, using the same
    // boundary condition throughout (default is `clamp`, which means "use the
    // explicit tangents")
    Path3(Eigen::Matrix<float, Eigen::Dynamic, 3> const& nodes,
          Eigen::RowVector3f tang_left, Eigen::RowVector3f tang_right,
          bdry_cond bc = bdry_cond::clamp);
    Path3(std::vector<glm::vec3> const& nodes, glm::vec3 tang_left,
          glm::vec3 tang_right, bdry_cond bc = bdry_cond::clamp);

    // Set tangents at left and right endpoints explicitly, and use a custom
    // boundary condition object for full flexibility (e.g. use the given
    // tangents only in certain dimensions)
    Path3(Eigen::Matrix<float, Eigen::Dynamic, 3> const& nodes,
          Eigen::RowVector3f tang_left, Eigen::RowVector3f tang_right,
          BdryConds3 const& bcs);
    Path3(std::vector<glm::vec3> const& nodes, Eigen::RowVector3f tang_left,
          Eigen::RowVector3f tang_right, BdryConds3 const& bcs);

    ~Path3() {}

    // Accessors
    const Eigen::Matrix<float, Eigen::Dynamic, 3> nodes() const {
        return nodes_;
    }
    Eigen::DenseIndex num_nodes() const { return nodes().rows(); }
    Eigen::DenseIndex num_pieces() const { return num_nodes() - 1; }
    const Eigen::Matrix<float, Eigen::Dynamic, 3> tangents() const {
        return tangents_;
    }
    const BdryConds3 bdry_conds() const { return bdry_conds_; }

    // Operator overloads
    Eigen::RowVector3f operator()(float param) const;
    Eigen::Matrix<float, Eigen::Dynamic, 3>
    operator()(Eigen::VectorXf const& params) const;

    // Utilities for constructing the system of equations for the tangents
    Eigen::MatrixXf system_matrix(bdry_cond bc_left, bdry_cond bc_right) const;
    Eigen::VectorXf system_rhs(bdry_cond bc_left, bdry_cond bc_right,
                               int dim) const;

    // Arc length stuff
    Eigen::VectorXf arc_length_lin_approx(size_t num_params) const;
    Eigen::VectorXf arc_length_params_lin_approx(size_t num_params) const;
    float total_length(size_t num_params) const;

    // Derivatives and derived vectors
    Eigen::RowVector3f deriv1(float param) const;
    Eigen::RowVector3f deriv2(float param) const;
    Eigen::RowVector3f deriv3(float param) const;
    Eigen::RowVector3f unit_tangent(float param) const;
    Eigen::RowVector3f unit_normal(float param) const;
    Eigen::RowVector3f unit_binormal(float param) const;

  private:
    const Eigen::Matrix<float, Eigen::Dynamic, 3> nodes_;
    const BdryConds3 bdry_conds_;
    Eigen::Matrix<float, Eigen::Dynamic, 3> tangents_;
    std::vector<Eigen::PartialPivLU<Eigen::MatrixXf>> sys_matrix_decomps_;
    std::vector<Eigen::VectorXf> sys_rhs_;
    Eigen::Matrix<float, Eigen::Dynamic, 3> a_vecs_;
    Eigen::Matrix<float, Eigen::Dynamic, 3> b_vecs_;

    // Internal helpers
    void compute_matrices_();
    void init_tangents_();
    void init_tangents_(Eigen::RowVector3f const& tang_left,
                        Eigen::RowVector3f const& tang_right);
    void init_rhs_();
    void compute_tangents_();
    void init_a_and_b_vecs_();
}; // class Path3

std::ostream& operator<<(std::ostream& out, Path3 const& p);

} // namespace tomovis
