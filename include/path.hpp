#pragma once

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <glm/glm.hpp>
#include <iostream>
#include <utility>
#include <vector>

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
    // TODO: use std::vector<Eigen::RowVector3f>?
    Path3(Eigen::Matrix<float, Eigen::Dynamic, 3> const& nodes,
          bdry_cond bc = bdry_cond::natural);
    Path3(Eigen::Matrix<float, Eigen::Dynamic, 3> const& nodes,
          BdryConds3 const& bcs);
    Path3(Eigen::Matrix<float, Eigen::Dynamic, 3> const& nodes,
          Eigen::RowVector3f tang_left, Eigen::RowVector3f tang_right,
          bdry_cond bc = bdry_cond::clamp);
    Path3(Eigen::Matrix<float, Eigen::Dynamic, 3> const& nodes,
          Eigen::RowVector3f tang_left, Eigen::RowVector3f tang_right,
          BdryConds3 const& bcs);

    ~Path3() {}

    const Eigen::Matrix<float, Eigen::Dynamic, 3> nodes() const {
        return nodes_;
    }
    Eigen::DenseIndex num_nodes() const { return nodes().rows(); }
    Eigen::DenseIndex num_pieces() const { return num_nodes() - 1; }
    const Eigen::Matrix<float, Eigen::Dynamic, 3> tangents() const {
        return tangents_;
    }
    const BdryConds3 bdry_conds() const { return bdry_conds_; }

    Eigen::RowVector3f operator()(float param) const;
    Eigen::Matrix<float, Eigen::Dynamic, 3>
    operator()(Eigen::VectorXf const& params) const;

    Eigen::MatrixXf system_matrix(bdry_cond bc_left, bdry_cond bc_right) const;
    Eigen::VectorXf system_rhs(bdry_cond bc_left, bdry_cond bc_right,
                               int dim) const;

    Eigen::VectorXf arc_length_lin_approx(size_t num_params) const;
    Eigen::VectorXf arc_length_params_lin_approx(size_t num_params) const;
    float total_length(size_t num_params) const;

  private:
    const Eigen::Matrix<float, Eigen::Dynamic, 3> nodes_;
    const BdryConds3 bdry_conds_;
    Eigen::Matrix<float, Eigen::Dynamic, 3> tangents_;
    std::vector<Eigen::PartialPivLU<Eigen::MatrixXf>> sys_matrix_decomps_;
    std::vector<Eigen::VectorXf> sys_rhs_;

    void compute_matrices_();
    void init_tangents_();
    void init_tangents_(Eigen::RowVector3f const& tang_left,
                        Eigen::RowVector3f const& tang_right);
    void init_rhs_();
    void compute_tangents_();
}; // class Path3

std::ostream& operator<<(std::ostream& out, Path3 const& p);

} // namespace tomovis
