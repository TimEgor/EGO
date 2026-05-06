#pragma once

#include "ComputeMatrix44.h"
#include "ComputeQuaternion.h"

namespace ego
{
	using TransformMatrix = ComputeMatrix4x4;

	struct Transform final
	{
		TransformMatrix m_matrix = ComputeMatrix4x4Identity;

		Transform() = default;
		Transform(const TransformMatrix& matrix) : m_matrix(matrix) {}

		const ComputeVector3& getAxisX() const { return m_matrix.getColumn(0).getXYZ(); }
		void setAxisX(const ComputeVector3& axis) { m_matrix.getColumn(0).setXYZ(axis); }
		const ComputeVector3& getAxisY() const { return m_matrix.getColumn(1).getXYZ(); }
		void setAxisY(const ComputeVector3& axis) { m_matrix.getColumn(1).setXYZ(axis); }
		const ComputeVector3& getAxisZ() const { return m_matrix.getColumn(2).getXYZ(); }
		void setAxisZ(const ComputeVector3& axis) { m_matrix.getColumn(2).setXYZ(axis); }
		const ComputeVector3& getOrigin() const { return m_matrix.getColumn(3).getXYZ(); }
		void setOrigin(const ComputeVector3& origin) { m_matrix.getColumn(3).setXYZ(origin); }

		ComputeMatrix3x3 getRotationMatrix() const;
		void getRotationMatrix(ComputeMatrix3x3& rotation) const;
		void setRotationMatrix(const ComputeMatrix3x3& rotation);

		ComputeQuaternion getRotationQuaternion() const;
		void getRotationQuaternion(ComputeQuaternion& rotation) const;
		void setRotationQuaternion(const ComputeQuaternion& rotation);
	};
}
    