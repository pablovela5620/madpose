#ifndef SOLVER_H
#define SOLVER_H

#include <colmap/util/types.h>
#include <PoseLib/solvers/p3p.h>
#include <PoseLib/misc/re3q3.h>

#include "pose.h"
#include "utils.h"

namespace acmpose {

PoseAndScale estimate_scale_and_pose(const Eigen::MatrixXd &X, const Eigen::MatrixXd &Y, const Eigen::VectorXd &W);

std::vector<PoseScaleOffset> estimate_scale_and_pose_with_offset_3pts_wrap(
    const Eigen::Matrix3d &x_homo, const Eigen::Matrix3d &y_homo,
    const Eigen::Vector3d &depth_x, const Eigen::Vector3d &depth_y);

std::vector<Eigen::Vector4d> solve_scale_and_shift(
    const Eigen::Matrix3d &x_homo, const Eigen::Matrix3d &y_homo, 
    const Eigen::Vector3d &depth_x, const Eigen::Vector3d &depth_y);

std::vector<Eigen::Vector<double, 5>> solve_scale_and_shift_shared_focal(
    const Eigen::Matrix3x4d &x_homo, const Eigen::Matrix3x4d &y_homo, 
    const Eigen::Vector4d &depth_x, const Eigen::Vector4d &depth_y);

std::vector<Eigen::Vector<double, 6>> solve_scale_and_shift_two_focal(
    const Eigen::Matrix3x4d &x_homo, const Eigen::Matrix3x4d &y_homo, 
    const Eigen::Vector4d &depth_x, const Eigen::Vector4d &depth_y);

int estimate_scale_shift_pose(
    const Eigen::Matrix3d &x_homo, const Eigen::Matrix3d &y_homo,
    const Eigen::Vector3d &depth_x, const Eigen::Vector3d &depth_y, 
    std::vector<PoseScaleOffset> *output, bool scale_on_x=false);

int estimate_scale_shift_pose_shared_focal(
    const Eigen::Matrix3x4d &x_homo, const Eigen::Matrix3x4d &y_homo,
    const Eigen::Vector4d &depth_x, const Eigen::Vector4d &depth_y, 
    std::vector<PoseScaleOffsetSharedFocal> *output, bool scale_on_x=false);

int estimate_scale_shift_pose_two_focal(
    const Eigen::Matrix3x4d &x_homo, const Eigen::Matrix3x4d &y_homo,
    const Eigen::Vector4d &depth_x, const Eigen::Vector4d &depth_y, 
    std::vector<PoseScaleOffsetTwoFocal> *output, bool scale_on_x=false);

std::vector<PoseScaleOffset> estimate_scale_shift_pose_wrapper(
    const Eigen::Matrix3d &x_homo, const Eigen::Matrix3d &y_homo,
    const Eigen::Vector3d &depth_x, const Eigen::Vector3d &depth_y);

std::vector<PoseScaleOffsetSharedFocal> estimate_scale_shift_pose_shared_focal_wrapper(
    const Eigen::Matrix3x4d &x_homo, const Eigen::Matrix3x4d &y_homo,
    const Eigen::Vector4d &depth_x, const Eigen::Vector4d &depth_y);

std::vector<PoseScaleOffsetTwoFocal> estimate_scale_shift_pose_two_focal_wrapper(
    const Eigen::Matrix3x4d &x_homo, const Eigen::Matrix3x4d &y_homo,
    const Eigen::Vector4d &depth_x, const Eigen::Vector4d &depth_y);

} // namespace acmpose

#endif 