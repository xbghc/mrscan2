#include "geometry_utils.h"
#include <cmath>

namespace geometry_utils{
    
    QMatrix4x4 rotateMatrix(const QVector3D& angle) {
        QMatrix4x4 r;
        r.rotate(angle.x(), QVector3D(1, 0, 0));
        r.rotate(angle.y(), QVector3D(0, 1, 0));
        r.rotate(angle.z(), QVector3D(0, 0, 1));
        return r;
    }
    
    Axis getAxis(const QVector3D& angle, const QVector3D& initVector) {
        // 将初始向量按给定角度旋转
        QVector3D rotatedVector = rotateMatrix(angle).map(initVector);
        
        // 取绝对值找到最大的分量
        double absX = std::abs(rotatedVector.x());
        double absY = std::abs(rotatedVector.y());
        double absZ = std::abs(rotatedVector.z());
        
        // 返回最大分量对应的轴
        if (absX >= absY && absX >= absZ) {
            return Axis::X;
        } else if (absY >= absZ) {
            return Axis::Y;
        } else {
            return Axis::Z;
        }
    }
    
    QVector3D applyRotate(const QVector3D& angle, const QVector3D& vector) {
        return rotateMatrix(angle).map(vector);
    }
    
    QPair<QVector3D, QVector3D> computePlaneIntersection(
        const double A1, const double B1, const double C1, const double D1,
        const double A2, const double B2, const double C2, const double D2) {
        
        auto n1 = QVector3D(A1, B1, C1);
        auto n2 = QVector3D(A2, B2, C2);

        QVector3D line_direction = QVector3D::crossProduct(n1, n2);
        constexpr double epsilon = 1e-9; // Adjust based on precision requirements

        // 1. Check if planes are parallel or coincident
        if (line_direction.lengthSquared() <
            epsilon * epsilon) { // Use square to avoid sqrt, epsilon also needs to be squared for comparison
            // Normal vectors are parallel, planes may be parallel or coincident
            // To distinguish between parallel but not coincident (no intersection line) and coincident (infinite intersection lines), further checks are needed:
            // For example, check if a point on one plane is on the other plane.
            // Or check if (A1,B1,C1,D1) and (A2,B2,C2,D2) are linearly related.
            // For this function, if the goal is to return a "unique" intersection line, both parallel and coincident cases are considered as no unique intersection line.
            return {QVector3D(), QVector3D()}; // Indicates no unique intersection line or error
        }

        QVector3D point_on_line;
        // Use plane equation in the form Ax + By + Cz = -D
        double d1_val = -D1;
        double d2_val = -D2;

        // 2. Calculate a point on the intersection line (more robust method)
        // By selecting the component with the largest absolute value in line_direction to decide which coordinate to set to 0,
        // to ensure that the determinant used in solving (i.e., the corresponding component of line_direction)
        // is not zero and is large, improving numerical stability.
        double absLx = std::abs(line_direction.x());
        double absLy = std::abs(line_direction.y());
        double absLz = std::abs(line_direction.z());

        if (absLx >= absLy && absLx >= absLz) { // line_direction.x() has the largest absolute value or one of them
            // Set x = 0. Solve:
            // B1*y + C1*z = d1_val
            // B2*y + C2*z = d2_val
            // det = B1*C2 - B2*C1 = line_direction.x()
            double det = line_direction.x();
            point_on_line.setX(0);
            point_on_line.setY((d1_val * C2 - d2_val * C1) / det);
            point_on_line.setZ((B1 * d2_val - B2 * d1_val) / det);
        } else if (absLy >= absLx &&
                   absLy >= absLz) { // line_direction.y() has the largest absolute value or one of them
            // Set y = 0. Solve:
            // A1*x + C1*z = d1_val
            // A2*x + C2*z = d2_val
            // det_sys = A1*C2 - A2*C1 = -line_direction.y()
            double det =
                -line_direction.y(); // (because line_direction.y() = A2*C1 - A1*C2)
            point_on_line.setY(0);
            point_on_line.setX((d1_val * C2 - d2_val * C1) / det);
            point_on_line.setZ((A1 * d2_val - A2 * d1_val) / det);
        } else { // line_direction.z() has the largest absolute value
            // Set z = 0. Solve:
            // A1*x + B1*y = d1_val
            // A2*x + B2*y = d2_val
            // det = A1*B2 - A2*B1 = line_direction.z()
            double det = line_direction.z();
            point_on_line.setZ(0);
            point_on_line.setX((d1_val * B2 - d2_val * B1) / det);
            point_on_line.setY((A1 * d2_val - A2 * d1_val) / det);
        }

        // Return a point on the intersection line and the normalized direction vector
        return {point_on_line, line_direction.normalized()};
    }
    
    QPair<QVector3D, QVector3D> computePlaneIntersection(
        const QVector3D& angle1, const QVector3D& offset1,
        const QVector3D& angle2, const QVector3D& offset2,
        const QVector3D& initNormalVector) {
        
        auto v1 = rotateMatrix(angle1).map(initNormalVector);
        auto v2 = rotateMatrix(angle2).map(initNormalVector);
        auto D1 = -(QVector3D::dotProduct(v1, offset1));
        auto D2 = -(QVector3D::dotProduct(v2, offset2));
        return computePlaneIntersection(v1.x(), v1.y(), v1.z(), D1, v2.x(), v2.y(), v2.z(), D2);
    }
    
}