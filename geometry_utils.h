#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QPair>

namespace geometry_utils{
    enum class Axis : int {
        X = 0,
        Y = 1,
        Z = 2
    };
    
    Axis getAxis(const QVector3D& vector);

    QVector3D rotateMatrix(const QVector3D& vector, const QMatrix4x4& matrix);
    
    /**
     * @brief Given angle, get the corresponding rotation matrix
     * @details Rotation is in fixed x,y,z order
     * @param angle QVector3D, corresponding to rotation angles of x,y,z axes respectively
     * @return Rotation matrix
     */
    QMatrix4x4 rotateMatrix(const QVector3D& angle);
    
    /**
     * @brief 根据角度和初始向量确定主要轴方向
     * @param angle 旋转角度
     * @param initVector 初始向量
     * @return 主要轴方向枚举
     * @details 将初始向量按给定角度旋转，然后判断旋转后的向量主要沿哪个坐标轴方向
     */
    Axis getAxis(const QVector3D& angle, const QVector3D& initVector);
    
    /**
     * @brief 对向量应用旋转变换
     * @param angle 旋转角度
     * @param vector 要旋转的向量
     * @return 旋转后的向量
     */
    QVector3D applyRotate(const QVector3D& angle, const QVector3D& vector);
    
    /**
     * @brief 计算两个平面的交线
     * @param A1, B1, C1, D1 第一个平面的方程系数 (A1*x + B1*y + C1*z + D1 = 0)
     * @param A2, B2, C2, D2 第二个平面的方程系数 (A2*x + B2*y + C2*z + D2 = 0)
     * @return QPair<QVector3D, QVector3D> 返回交线上的一点和交线的方向向量
     * @details 如果返回的方向向量长度为0，表示两个平面平行，没有交线
     */
    QPair<QVector3D, QVector3D> computePlaneIntersection(
        const double A1, const double B1, const double C1, const double D1,
        const double A2, const double B2, const double C2, const double D2);
    
    /**
     * @brief 计算两个平面的交线（通过角度和偏移定义平面）
     * @param angle1 第一个平面的旋转角度
     * @param offset1 第一个平面的偏移
     * @param angle2 第二个平面的旋转角度  
     * @param offset2 第二个平面的偏移
     * @param initNormalVector 初始法向量
     * @return QPair<QVector3D, QVector3D> 返回交线上的一点和交线的方向向量
     */
    QPair<QVector3D, QVector3D> computePlaneIntersection(
        const QVector3D& angle1, const QVector3D& offset1,
        const QVector3D& angle2, const QVector3D& offset2,
        const QVector3D& initNormalVector);
}

#endif // GEOMETRY_UTILS_H
