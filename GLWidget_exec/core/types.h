#pragma once
#include "vrt.h"
#include <iostream>
#include <QVector3D>
#include <glog/logging.h>
#include <Eigen/Eigen>
#include "vrt.h"
namespace vrt {
	template <typename T>
	inline bool isNaN(const T x) {
		return std::isnan(x);
	}
	template <>
	inline bool isNaN(const int x) {
		return false;
	}

	/**
	 * @class 封装底层数据的接口，目前底层数据采用eigen库
	 */
	template <class dataT,class typeT>
	class TypeWithData 
	{
	public:
		TypeWithData(const dataT& data):dat(data){}
		TypeWithData() = default;
		typeT operator+(const typeT &v) const {
			DCHECK(!v.HasNaNs());
			return typeT(dat + v.dat);
		}
		typeT operator-(const typeT &v) const {
			DCHECK(!v.HasNaNs());
			return typeT(dat - v.dat);
		}
		template <typename U>
		typeT operator*(U f) const {
			return typeT(dat*f);
		}
		template <typename U>
		typeT operator/(U f) const {
			CHECK_NE(f, 0);
			return typeT(dat/f);
		}
		//#PERF1 下面这些类型转换后续需要优化，可以把模板类换成宏定义
		typeT& operator+=(const typeT &v) const {
			DCHECK(!v.HasNaNs());
			dat += v.dat;
			return *(typeT*)(this);
		}
		typeT& operator-=(const typeT &v) const {
			DCHECK(!v.HasNaNs());
			dat -= v.dat;
			return *(typeT*)(this);
		}
		template <typename U>
		typeT& operator*=(U f) {
			DCHECK(!isNaN(f));
			dat *= f;
			return *(typeT*)(this);
		}
		template <typename U>
		typeT &operator/=(U f) {
			CHECK_NE(f, 0);
			dat /= f;
			return *(typeT*)(this);
		}
		bool operator==(const typeT &v) const {
			DCHECK(!v.HasNaNs());
			dat -= v.dat;
			return *(typeT*)(this);
		}

		bool operator!=(const typeT &v) const {
			DCHECK(!v.HasNaNs());
			dat -= v.dat;
			return *(typeT*)(this);
		}

		typeT& operator-() const {
			dat = -dat;
			return *(typeT*)(this);
		}
	protected:
		dataT dat;
	};

	/**
	 * @class 针对向量类进一步进行封装
	 */
	template <typename elemT, typename dataT, typename typeT, int Size>
	class TypeWithSizedRawData : public TypeWithData<dataT, typeT> {
	};

	template <typename elemT, typename dataT, typename typeT>
	class TypeWithSizedRawData <typename elemT, typename dataT, typename typeT, 2>: public TypeWithData<dataT, typeT>
	{
	public:
		using TypeWithData<dataT, typeT>::TypeWithData;
		TypeWithSizedRawData() 
			: TypeWithData<dataT, typeT>() { dat << 0, 0; }
		TypeWithSizedRawData(elemT xx, elemT yy)
			: TypeWithData<dataT, typeT>()
		{
			dat << xx, yy;
			DCHECK(!HasNaNs());
		}
		TypeWithSizedRawData(elemT val) {
			dat << val, val;
			DCHECK(!HasNaNs());
		}
		bool HasNaNs() {
			return isNaN((*this)[0]) || isNaN((*this)[1]);
		}
		elemT operator[](int i) const {
			DCHECK(i >= 0 && i <= 1);
			return dat[i];
		}
		elemT &operator[](int i) {
			DCHECK(i >= 0 && i <= 1);
			return dat[i];
		}
		elemT& x() {
			return dat.x();
		}
		elemT& y() {
			return dat.y();
		}
		const elemT& x() const {
			return dat.x();
		}
		const elemT& y() const {
			return dat.y();
		}
		elemT dot(const typeT& rhs)const {
			return dat.dot(rhs.dat);
		}
		typeT cross(const typeT& rhs) const {
			return typeT(dat.cross(rhs.dat));
		}
		typeT normalize() const {
			return typeT(dat / length());
		}
		elemT lengthSquared() const { return x() * x() + y() * y(); }
		elemT length() const { return std::sqrt(LengthSquared()); }
	};

	template <typename elemT, typename dataT, typename typeT>
	class TypeWithSizedRawData <typename elemT, typename dataT, typename typeT, 3>: public TypeWithData<dataT, typeT>
	{
	public:
		using TypeWithData<dataT, typeT>::TypeWithData;
		TypeWithSizedRawData() 
			: TypeWithData<dataT, typeT>() { dat << 0, 0, 0; }

		TypeWithSizedRawData(elemT xx, elemT yy, elemT zz)
			: TypeWithData<dataT, typeT>() 
		{
			dat << xx, yy, zz;
			DCHECK(!HasNaNs());
		}
		TypeWithSizedRawData(elemT val) {
			dat << val, val, val;
			DCHECK(!HasNaNs());
		}
		bool HasNaNs() {
			return isNaN((*this)[0]) || isNaN((*this)[1]) || isNaN((*this)[2]);
		}
		elemT operator[](int i) const {
			DCHECK(i >= 0 && i <= 2);
			return dat[i];
		}
		elemT &operator[](int i) {
			DCHECK(i >= 0 && i <= 2);
			return dat[i];
		}
		elemT& x() {
			return dat.x();
		}
		elemT& y() {
			return dat.y();
		}
		elemT& z() {
			return dat.z();
		}
		const elemT& x() const {
			return dat.x();
		}
		const elemT& y() const {
			return dat.y();
		}
		const elemT& z() const {
			return dat.z();
		}
		elemT dot(const typeT& rhs)const {
			return dat.dot(rhs.dat);
		}
		typeT cross(const typeT& rhs) const {
			return typeT(dat.cross(rhs.dat));
		}
		void normalize(){
			(*this) /= length();
		}
		elemT lengthSquared() const { return x() * x() + y() * y() + z() * z(); }
		elemT length() const { return std::sqrt(lengthSquared()); }
	};

	template<class T>
	T  Normalize(const T& rhs) {
		return rhs / rhs.length();
	}

	// 将向量、点、法向以不同类进行封装，主要目的是为了区分不同的仿射变换逻辑
	// Vector Declarations
	template <typename T>
	class Vector2: public TypeWithSizedRawData<T,Eigen::Matrix<T, 2, 1>,Vector2<T>,2 >
	{
	public:
		typedef TypeWithSizedRawData<T, Eigen::Matrix<T, 2, 1>, Vector2<T>, 2 > father_t;
		using father_t::father_t;
		Vector2() :father_t() {};
	};

	template <typename T>
	inline std::ostream &operator<<(std::ostream &os, const Vector2<T> &v) {
		os << "[ " << v[0] << ", " << v[1] << " ]";
		return os;
	}

	template <typename T>
	class Vector3 : public TypeWithSizedRawData<T,Eigen::Matrix<T, 3, 1>, Vector3<T>, 3 >
	{
	public:
		typedef TypeWithSizedRawData<T, Eigen::Matrix<T, 3, 1>, Vector3<T>, 3 > father_t;
		using father_t::father_t;
		Vector3() :father_t() {};

		
		Vector3(const Normal3<T> &n)
			:father_t(n.x(), n.y(), n.z())
		{
			DCHECK(!n.HasNaNs());
		}

		explicit operator QVector3D() const { return QVector3D(x(), y(), z()); }

		Vector3(std::initializer_list<T> list)
		{
			auto it = list.begin();
			x() = *it;
			y() = *(++it);
			z() = *(++it);
		}
	};

	template <typename T>
	inline std::ostream &operator<<(std::ostream &os, const Vector3<T> &v) {
		os << "[ " << v.x() << ", " << v.y() << ", " << v.z() << " ]";
		return os;
	}

	// Point Declarations
	template <typename T>
	class Point2 : public TypeWithSizedRawData<T, Eigen::Matrix<T, 2, 1>, Point2<T>, 2 > 
	{
	public:
		typedef TypeWithSizedRawData<T, Eigen::Matrix<T, 2, 1>, Point2<T>, 2 >  father_t;
		using father_t::father_t;
		Point2() :father_t() {};

		Vector2<T> operator-(const Point2<T>& rhs) {
			return Vector2<T>(dat - rhs.dat);
		}
	};

	template <typename T>
	inline std::ostream &operator<<(std::ostream &os, const Point2<T> &v) {
		os << "[ " << v.x() << ", " << v.y()<< " ]";
		return os;
	}

	template <typename T>
	class Point3 : public TypeWithSizedRawData<T, Eigen::Matrix<T, 3, 1>, Point3<T>, 3 > {
	public:
		typedef TypeWithSizedRawData<T, Eigen::Matrix<T, 3, 1>, Point3<T>, 3 > father_t;
		using father_t::father_t;
		Point3() :father_t() {};

		template <typename U>
		explicit Point3(const Point3<U> &p)
		{
			dat << p.x(), p.y(), p.z();
			DCHECK(!HasNaNs());
		}

		Point3(const QVector3D& qvec)
		{
			dat << qvec.x(), qvec.y(), qvec.z();
		}

		explicit operator QVector3D() const { return QVector3D(x(), y(), z()); }
		template <typename U>
		explicit operator Vector3<U>() const {
			return Vector3<U>(x(), y(), z());
		}

		Vector3<T> operator-(const Point3<T>& rhs) const{
			return Vector3<T>(dat - rhs.dat);
		}
	};

	template <typename T>
	inline std::ostream &operator<<(std::ostream &os, const Point3<T> &v) {
		os << "[ " << v.x() << ", " << v.y() << ", " << v.z() << " ]";
		return os;
	}

	// Normal Declarations
	template <typename T>
	class Normal3 : public TypeWithSizedRawData<T, Eigen::Matrix<T, 3, 1>, Normal3<T>, 3 > {
	public:
		typedef TypeWithSizedRawData<T, Eigen::Matrix<T, 3, 1>, Normal3<T>, 3 > father_t;
		using father_t::father_t;
		Normal3() :father_t() {};

		explicit Normal3<T>(const Vector3<T> &v) {
			dat << v.x(), v.y(), v.z();
			DCHECK(!v.HasNaNs());
		}
	};

	template <typename T>
	inline std::ostream &operator<<(std::ostream &os, const Normal3<T> &v) {
		os << "[ " << v.x() << ", " << v.y() << ", " << v.z() << " ]";
		return os;
	}

	typedef Normal3<Float> Normal3f;

	// Geometry Inline Functions
	template <typename T, typename U>
	inline Vector3<T> operator*(U s, const Vector3<T> &v) {
		return v * s;
	}

	template <typename T, typename U>
	inline Vector2<T> operator*(U f, const Vector2<T> &v) {
		return v * f;
	}

	template <typename T, typename U>
	inline Point3<T> operator*(U f, const Point3<T> &p) {
		DCHECK(!p.HasNaNs());
		return p * f;
	}

	template <typename T, typename U>
	inline Point2<T> operator*(U f, const Point2<T> &p) {
		DCHECK(!p.HasNaNs());
		return p * f;
	}

	template <typename T, typename U>
	inline Normal3<T> operator*(U f, const Normal3<T> &n) {
		return Normal3<T>(f * n.x, f * n.y, f * n.z);
	}

	template <typename T>
	inline Point3<T> operator+(const Point3<T> & pt, const Vector3<T> & vec) {
		return Point3<T>(pt.x() + vec.x(), pt.y() + vec.y(), pt.z() + vec.z());
	}

	template <typename T>
	inline Point3<T> operator+(const Vector3<T> & vec, const Point3<T> & pt) {
		return Point3<T>(pt.x() + vec.x(), pt.y() + vec.y(), pt.z() + vec.z());
	}

	//global typedef
	typedef Vector2<Float> Vector2f;
	typedef Vector2<int> Vector2i;
	typedef Vector3<Float> Vector3f;
	typedef Vector3<int> Vector3i;
	typedef Point2<Float> Point2f;
	typedef Point2<int> Point2i;
	typedef Point3<Float> Point3f;
	typedef Point3<int> Point3i;
	typedef Vector3f Vec3f;
	typedef Point3f PType3f;
	typedef Vec3f ColorType;
	template<class T> class ListElement;

	//链表类
	template<class T>
	class ListElementIterator {
	public:
		ListElementIterator(ListElement<T>* elem)
			:elem_(elem->get()) {}
		ListElementIterator<T>& operator++() {
			elem_ = elem_->next;
			return *this;
		}
		ListElementIterator<T> operator++(int) {
			T* pevElem = elem_;
			elem_ = elem_->next;
			return ListElementIterator (pevElem);
		}
		ListElementIterator<T>& operator--() {
			elem_ = elem_->prev;
			return *this;
		}
		ListElementIterator<T> operator--(int) {
			T* nxtElem = elem_;
			elem_ = elem_->prev;
			return ListElementIterator(nxtElem);
		}
		bool operator==(const ListElementIterator<T>& rhs) const{
			return elem_ == rhs.elem_;
		}
		bool operator!=(const ListElementIterator<T>& rhs) const {
			return elem_ != rhs.elem_;
		}
		ListElementIterator<T>& operator=(const ListElementIterator<T>& rhs) {
			elem_ = rhs.elem_;
			return *this;
		}
		T*& operator*() {
			return elem_;
		}
		explicit operator bool() {
			return bool(this->elem_);
		}
		bool operator!() {
			return !(elem_);
		}
	private:
		T* elem_;
	};

	template<class T>
	class ListElement {
	public:
		typedef ListElementIterator<T> iterator;
		iterator begin() {
			ListElement<T>* it = this;
			while (it->prev)
			{
				it = it->prev;
			}
			return iterator(it);
		}

		iterator end() {
			return iterator(nullptr);
		}

		void printList() {
			for (auto it = begin(); it != end() ;it++)
			{
				qDebug() << (*it);
			}
		}

		iterator before_end() {
			ListElement<T>* it = this;
			while (it->next)
			{
				it = it->next;
			}
			return iterator(it);
		}

		int size() {
			int i = 0;
			for (auto it = begin(); it != end(); it++, i++);
			return i;
		}

		T* get() {
			return (T*)this;
		}

		void push_back(T* val) {
			if (val) {
				val->del();
				iterator it = before_end();
				val->prev = *it;
				val->next = nullptr;
				(*it)->next = val;
			}
		}

		void del() {
			if (prev) prev->next = next;
			if (next) next->prev = prev;
			this->prev = nullptr;
			this->next = nullptr;
		}
		bool findInList(iterator elem) {
			if (!elem) return false;
			for (auto it = this->begin(); it != this->end(); it++) {
				if (elem == it) return true;
			}
			return false;
		}
		~ListElement() {
			del();
		}
		friend iterator;
		T* Prev() const { return prev; }
		T* Next() const { return next; }
	private:
		T* prev = nullptr, *next = nullptr;
	};

	template<class T>
	std::istream& operator>>(std::iostream& lhs,Vector3<T>& rhs) {
		T x, y, z;
		char c;
		std::istream& ret = (lhs >> x >> c >> y >> c >> z);
		if (ret) {
			rhs = Vector3<T>(x, y, z);
		}
		return ret;
	}

	template<class T>
	std::istream& operator>>(std::iostream& lhs, Point3<T>& rhs) {
		T x, y, z;
		char c;
		auto& ret = (lhs >> x >> c >> y >> c >> z);
		if (ret) {
			rhs = Point3<T>(x, y, z);
		}
		return ret;
	}

	inline std::istream& operator>>(std::iostream& lhs, std::vector<std::vector<Point3f>>& rhs) {
		std::string str;
		std::istream& is = std::getline(lhs,str,'}');
		int idx = str.find('{');
		if (idx == -1) {  //return error iostream
			lhs.clear();
			return lhs>>str;
		}
		str = str.substr(idx+1);
		bool flag = false;
		while (true) {
			int idx1 = str.find('(');
			if (idx1 == -1) break;
			int idx2 = str.find(')');
			if (idx2 == -1) {	//return error iostream
				lhs.clear();
				return lhs >> str;
			}
			rhs.push_back(std::vector<Point3f>());
			std::string subs = str.substr(idx1 + 1, idx2 - idx1 - 1);
			str = str.substr(idx2 + 1);
			std::stringstream sin(subs);
			Point3f pt;
			while (sin>>pt) {
				rhs.back().push_back(pt);
			}
			if (rhs.back().empty()) DLOG(WARNING) << "blank Point3f vector input!";
		}
		return is;
	}

	//#TODO  包围盒类
	template<typename T>
	class Bounds3 {

	};

	typedef Bounds3<Float> Bounds3f;
}