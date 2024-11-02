#ifndef HYBRID_POSE_SHARED_FOCAL_ESTIMATION_H
#define HYBRID_POSE_SHARED_FOCAL_ESTIMATION_H

#include <RansacLib/ransac.h>
#include "optimizer.h"
#include "hybrid_ransac.h"
#include "solver.h"

namespace acmpose {

class HybridPoseSharedFocalEstimator {
public:
    HybridPoseSharedFocalEstimator(const std::vector<Eigen::Vector2d> &x0_norm, const std::vector<Eigen::Vector2d> &x1_norm,
                        const std::vector<double> &depth0, const std::vector<double> &depth1, 
                        const Eigen::Vector2d &min_depth, 
                        const double &norm_scale = 1.0,
                        const double &sampson_squared_weight = 1.0,
                        const std::vector<double> &squared_inlier_thresholds = {},
                        const std::vector<double> &uncert_weight = {}) : 
                        sampson_squared_weight_(sampson_squared_weight), norm_scale_(norm_scale),
                        min_depth_(min_depth), squared_inlier_thresholds_(squared_inlier_thresholds) {
                            assert(x0_norm.size() == x1_norm.size() && x0_norm.size() == depth0.size() && x0_norm.size() == depth1.size());
                            
                            if (uncert_weight.empty()) {
                                uncert_weight_ = Eigen::VectorXd::Ones(x0_norm.size());
                            } else {
                                assert(uncert_weight.size() == x0_norm.size());
                                uncert_weight_ = Eigen::Map<const Eigen::VectorXd>(uncert_weight.data(), uncert_weight.size());
                            }

                            d0_ = Eigen::Map<const Eigen::VectorXd>(depth0.data(), depth0.size());
                            d1_ = Eigen::Map<const Eigen::VectorXd>(depth1.data(), depth1.size());

                            x0_norm_ = Eigen::MatrixXd(3, x0_norm.size());
                            x1_norm_ = Eigen::MatrixXd(3, x1_norm.size());
                            for (int i = 0; i < x0_norm.size(); i++) {
                                x0_norm_.col(i) = x0_norm[i].homogeneous();
                                x1_norm_.col(i) = x1_norm[i].homogeneous();
                            }
                        }  

    ~HybridPoseSharedFocalEstimator() {}

    inline int num_minimal_solvers() const { return 2; }

    inline int min_sample_size() const { return 6; }

    void min_sample_sizes(std::vector<std::vector<int>>* min_sample_sizes) const {
        min_sample_sizes->resize(2);
        min_sample_sizes->at(0) = {4, 0};
        min_sample_sizes->at(1) = {0, 6};
    }

    inline int num_data_types() const { return 2; }

    void num_data(std::vector<int>* num_data) const {
        num_data->resize(2);
        num_data->at(0) = x0_norm_.cols();
        num_data->at(1) = x0_norm_.cols();
    }

    void solver_probabilities(std::vector<double>* probabilities) const {
        probabilities->resize(2);
        probabilities->at(0) = 1.0;
        probabilities->at(1) = 1.0;
    }

    inline int non_minimal_sample_size() const { return 36; }

    double GetWeight(int i) const { return uncert_weight_(i); }

    int MinimalSolver(const std::vector<std::vector<int>>& sample,
                      const int solver_idx, std::vector<PoseScaleOffsetSharedFocal>* models) const;

    // Returns 0 if no model could be estimated and 1 otherwise.
    // Implemented by a simple linear least squares solver.
    int NonMinimalSolver(const std::vector<std::vector<int>>& sample, const int solver_idx, PoseScaleOffsetSharedFocal* model) const;

    // Evaluates the line on the i-th data point.
    double EvaluateModelOnPoint(const PoseScaleOffsetSharedFocal& model, int t, int i, double squared_thres) const;

    // Linear least squares solver. 
    void LeastSquares(const std::vector<std::vector<int>>& sample, const int solver_idx, PoseScaleOffsetSharedFocal* model, bool final=false) const;

protected:
    Eigen::MatrixXd x0_norm_, x1_norm_;
    Eigen::VectorXd d0_, d1_;
    Eigen::Vector2d min_depth_; 
    Eigen::VectorXd uncert_weight_;
    double sampson_squared_weight_;

    std::vector<double> squared_inlier_thresholds_;
    double norm_scale_;
};

class HybridPoseSharedFocalEstimator3 : public HybridPoseSharedFocalEstimator {
public:
    HybridPoseSharedFocalEstimator3(const std::vector<Eigen::Vector2d> &x0_norm, const std::vector<Eigen::Vector2d> &x1_norm,
                         const std::vector<double> &depth0, const std::vector<double> &depth1, 
                         const Eigen::Vector2d &min_depth, 
                         const double &norm_scale = 1.0,
                         const double &sampson_squared_weight = 1.0,
                         const std::vector<double> &squared_inlier_thresholds = {},
                         const std::vector<double> &uncert_weight = {}) : 
                            HybridPoseSharedFocalEstimator(x0_norm, x1_norm, depth0, depth1, min_depth,
                                norm_scale, sampson_squared_weight, squared_inlier_thresholds, uncert_weight) {}

    void min_sample_sizes(std::vector<std::vector<int>>* min_sample_sizes) const {
        min_sample_sizes->resize(2);
        min_sample_sizes->at(0) = {4, 4, 0};
        min_sample_sizes->at(1) = {0, 0, 6};
    }

    inline int num_data_types() const { return 3; }

    void num_data(std::vector<int>* num_data) const {
        num_data->resize(3);
        num_data->at(0) = x0_norm_.cols();
        num_data->at(1) = x0_norm_.cols();
        num_data->at(2) = x0_norm_.cols();
    }

    int MinimalSolver(const std::vector<std::vector<int>>& sample,
                      const int solver_idx, std::vector<PoseScaleOffsetSharedFocal>* models) const {
        std::vector<std::vector<int>> sample_2 = {sample[0], sample[2]};
        return HybridPoseSharedFocalEstimator::MinimalSolver(sample_2, solver_idx, models);
    }

    // Returns 0 if no model could be estimated and 1 otherwise.
    // Implemented by a simple linear least squares solver.
    int NonMinimalSolver(const std::vector<std::vector<int>>& sample, const int solver_idx, PoseScaleOffsetSharedFocal* model) const;

    // Evaluates the line on the i-th data point.
    double EvaluateModelOnPoint(const PoseScaleOffsetSharedFocal& model, int t, int i, double squared_thres, bool gradcut=false) const;

    // Linear least squares solver. 
    void LeastSquares(const std::vector<std::vector<int>>& sample, const int solver_idx, PoseScaleOffsetSharedFocal* model, bool final=false) const;
};

class SharedFocalOptimizer3 {
protected:
    const Eigen::MatrixXd &x0_, &x1_;
    const Eigen::VectorXd &d0_, &d1_;
    Eigen::Vector4d qvec_;
    Eigen::Vector3d tvec_;
    Eigen::VectorXd uncert_weight_;
    double focal_;
    double scale_, offset0_, offset1_;
    Eigen::Vector2d min_depth_; 
    SharedFocalOptimizerConfig config_;

    const std::vector<int> &indices_reproj_0_, &indices_reproj_1_;
    const std::vector<int> &indices_sampson_;

    // ceres
    std::unique_ptr<ceres::Problem> problem_;
    ceres::Solver::Summary summary_;

    bool final_;
public:
    SharedFocalOptimizer3(const Eigen::MatrixXd &x0, const Eigen::MatrixXd &x1, 
                          const Eigen::VectorXd &depth0, const Eigen::VectorXd &depth1,
                          const std::vector<int> &indices_reproj_0, const std::vector<int> &indices_reproj_1, 
                          const std::vector<int> &indices_sampson,
                          const Eigen::Vector2d &min_depth,  
                          const PoseScaleOffsetSharedFocal &pose, 
                          const Eigen::VectorXd &uncert_weight,
                          const SharedFocalOptimizerConfig& config = SharedFocalOptimizerConfig(), 
                          bool final = false) : 
                     x0_(x0), x1_(x1), d0_(depth0), d1_(depth1), uncert_weight_(uncert_weight),
                     indices_reproj_0_(indices_reproj_0), indices_reproj_1_(indices_reproj_1), indices_sampson_(indices_sampson),
                     min_depth_(min_depth), config_(config), final_(final) {
        qvec_ = RotationMatrixToQuaternion<double>(pose.R());
        tvec_ = pose.t();
        offset0_ = pose.offset0;
        offset1_ = pose.offset1;
        scale_ = pose.scale;
        focal_ = pose.focal;
    }

    void SetUp() {
        problem_.reset(new ceres::Problem(config_.problem_options));

        ceres::LossFunction* geo_loss_func = config_.geom_loss_function.get();
        ceres::LossFunction* proj_loss_func = config_.reproj_loss_function.get();
        ceres::LossFunction* sampson_loss_func = config_.sampson_loss_function.get();
        if (geo_loss_func == nullptr) {
            geo_loss_func = new ceres::TrivialLoss();
        }
        if (proj_loss_func == nullptr) {
            proj_loss_func = new ceres::TrivialLoss();
        }
        if (sampson_loss_func == nullptr) {
            sampson_loss_func = new ceres::TrivialLoss();
        }
        // geo_loss_func = new ceres::ScaledLoss(geo_loss_func, config_.weight_geometric, ceres::DO_NOT_TAKE_OWNERSHIP);
        // sampson_loss_func = new ceres::ScaledLoss(sampson_loss_func, config_.weight_sampson, ceres::DO_NOT_TAKE_OWNERSHIP);

        if (config_.use_reprojection) {
            for (auto &i : indices_reproj_0_) {
                ceres::LossFunction* weighted_loss = new ceres::ScaledLoss(proj_loss_func, uncert_weight_(i), ceres::DO_NOT_TAKE_OWNERSHIP);
                ceres::CostFunction* reproj_cost_0 = LiftProjectionSharedFocalFunctor0::Create(x0_.col(i), x1_.col(i), d0_(i));
                problem_->AddResidualBlock(reproj_cost_0, weighted_loss, &offset0_, qvec_.data(), tvec_.data(), &focal_);
            }
            for (auto &i : indices_reproj_1_) {
                ceres::LossFunction* weighted_loss = new ceres::ScaledLoss(proj_loss_func, uncert_weight_(i), ceres::DO_NOT_TAKE_OWNERSHIP);
                ceres::CostFunction* reproj_cost_1 = LiftProjectionSharedFocalFunctor1::Create(x1_.col(i), x0_.col(i), d1_(i));
                problem_->AddResidualBlock(reproj_cost_1, weighted_loss, &scale_, &offset1_, qvec_.data(), tvec_.data(), &focal_);
            }
        }

        for (auto &i : indices_sampson_) {
            if (config_.use_sampson) {
                ceres::LossFunction* weighted_loss = new ceres::ScaledLoss(sampson_loss_func, uncert_weight_(i), ceres::DO_NOT_TAKE_OWNERSHIP);
                ceres::CostFunction* sampson_cost = SampsonErrorSharedFocalFunctor::Create(x0_.col(i), x1_.col(i), config_.weight_sampson);
                problem_->AddResidualBlock(sampson_cost, weighted_loss, qvec_.data(), tvec_.data(), &focal_);
            }
        }

        if (problem_->HasParameterBlock(&scale_)) {
            problem_->SetParameterLowerBound(&scale_, 0, 1e-2); // scale >= 0
        }
        if (problem_->HasParameterBlock(&offset0_)) {
            problem_->SetParameterLowerBound(&offset0_, 0, -min_depth_(0) + 1e-2); // offset0 >= -min_depth_(0)
        }
        if (problem_->HasParameterBlock(&offset1_)) {
            problem_->SetParameterLowerBound(&offset1_, 0, -min_depth_(1) + 1e-2); // offset1 >= -min_depth_(1)
        }

        if (problem_->HasParameterBlock(qvec_.data())) {
            if (config_.constant_pose) {
                problem_->SetParameterBlockConstant(qvec_.data());
                problem_->SetParameterBlockConstant(tvec_.data());
            }
            else {
            #ifdef CERES_PARAMETERIZATION_ENABLED
                ceres::LocalParameterization* quaternion_parameterization = 
                    new ceres::QuaternionParameterization;
                problem_->SetParameterization(qvec_.data(), quaternion_parameterization);
            #else
                ceres::Manifold* quaternion_manifold = 
                    new ceres::QuaternionManifold;
                problem_->SetManifold(qvec_.data(), quaternion_manifold);
            #endif
            }
        }
    }

    bool Solve() {
        if (problem_->NumResiduals() == 0) return false;
        ceres::Solver::Options solver_options = config_.solver_options;
    
        solver_options.linear_solver_type = ceres::DENSE_QR;

        solver_options.num_threads = 1; 
        // colmap::GetEffectiveNumThreads(solver_options.num_threads);
        #if CERES_VERSION_MAJOR < 2
        solver_options.num_linear_solver_threads =
            colmap::GetEffectiveNumThreads(solver_options.num_linear_solver_threads);
        #endif  // CERES_VERSION_MAJOR

        std::string solver_error;
        CHECK(solver_options.IsValid(&solver_error)) << solver_error;

        ceres::Solve(solver_options, problem_.get(), &summary_);
        return true;
    }

    PoseScaleOffsetSharedFocal GetSolution() {
        Eigen::Matrix3d R = QuaternionToRotationMatrix<double>(qvec_);
        return PoseScaleOffsetSharedFocal(R, tvec_, scale_, offset0_, offset1_, focal_);
    }
};

std::pair<PoseScaleOffsetSharedFocal, ransac_lib::HybridRansacStatistics> 
HybridEstimatePoseScaleOffsetSharedFocal(const std::vector<Eigen::Vector2d> &x0_norm, const std::vector<Eigen::Vector2d> &x1_norm,
                              const std::vector<double> &depth0, const std::vector<double> &depth1, 
                              const Eigen::Vector2d &min_depth,
                              const Eigen::Vector2d &pp0, const Eigen::Vector2d &pp1,
                              const ExtendedHybridLORansacOptions& options, 
                              const std::vector<double> &uncert_weights = {});

} // namespace acmpose

#endif // HYBRID_POSE_SHARED_FOCAL_ESTIMATION_H
