#pragma once
#include <vector>
#include <string>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
namespace vrt {
	inline bool compileVrtShader(QOpenGLShaderProgram& shaderProgram, const char* vs, const char* fs) {
		bool success = shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
		if (!success) {
			qDebug() << "Mesh: shaderProgram addShaderFromSourceFile failed!" << shaderProgram.log();
			return false;
		}
		success = shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
		if (!success) {
			qDebug() << "Mesh: shaderProgram addShaderFromSourceFile failed!" << shaderProgram.log();
			return false;
		}
		success = shaderProgram.link();
		if (!success) {
			qDebug() << "Mesh: shaderProgram link failed!" << shaderProgram.log(); 
			return false;
		}
		return true;
	}

	inline bool compileVrtShader(QOpenGLShaderProgram& shaderProgram, const char* vs, const char* gs, const char* fs) {
		bool success = shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
		if (!success) {
			qDebug() << "Mesh: shaderProgram addShaderFromSourceFile failed!" << shaderProgram.log();
			return false;
		}
		success = shaderProgram.addShaderFromSourceCode(QOpenGLShader::Geometry, gs);
		if (!success) {
			qDebug() << "Mesh: shaderProgram addShaderFromSourceFile failed!" << shaderProgram.log();
			return false;
		}
		success = shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
		if (!success) {
			qDebug() << "Mesh: shaderProgram addShaderFromSourceFile failed!" << shaderProgram.log();
			return false;
		}
		success = shaderProgram.link();
		if (!success) {
			qDebug() << "Mesh: shaderProgram link failed!" << shaderProgram.log();
			return false;
		}
		return true;
	}

	/* Some template utilities */
	// 类型名称转字符串
	template <typename T>
	struct TypeName
	{
		static const char* Get()
		{
			return typeid(T).name();
		}
	};

	// 通过实参tuple调用给定函数
	template<typename Func, typename Tup>
	decltype(auto) invoke(Func&& func, Tup&& t)
	{
		constexpr auto size = std::tuple_size<typename std::decay<Tup>::type>::value;
		// 创建参数个数size大小的模版数组
		return invoke_impl(std::forward<Func>(func), std::forward<Tup>(t), std::make_index_sequence<size>{});
	}

	template<typename Func, typename Tup, std::size_t... Index>
	decltype(auto) invoke_impl(Func&& func, Tup&& t, std::index_sequence<Index...>)
	{
		// Index为之前打包的size大小模版数组0,1,2,3。解包扩展后即成为size个参数
		return func(std::get<Index>(std::forward<Tup>(t))...);
	}

	//需要先进行模版声明，才能进行下面的特化
	template<typename T>
	struct function_traits {};

	// 获取函数信息
	template<typename R, typename ...Args>
	struct function_traits<std::function<R(Args...)>>
	{
		function_traits() = delete;
		static const size_t nargs = sizeof...(Args); // 形参数量

		typedef R result_type; // 返回类型
		typedef std::tuple<Args...> argTuple_t; //形参tuple类型

		// 输入参数类型,i为从0开始的参数类型索引
		template <size_t i>
		struct arg
		{
			typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
		};

		// 构造函数实参tuple
		template <int CountLast, int ArgCount, typename ArgsTupel>
		struct TupleConstructor
		{
			typedef typename std::tuple_element<ArgCount - CountLast, ArgsTupel>::type argtype;
			// 通过ios输入构造函数实参
			static int construct(ArgsTupel& args, std::iostream& ios) {
				argtype aArg;
				if (!(ios >> aArg)) return -1;
				std::get<ArgCount - CountLast>(args) = aArg;
				return TupleConstructor<CountLast - 1, ArgCount, ArgsTupel>::construct(args, ios);
			}
			// 解析函数形参类型字符串数组
			static void getArgNames(std::vector<std::string>& namevec) {
				namevec.push_back(TypeName<argtype>::Get());
				return TupleConstructor<CountLast - 1, ArgCount, ArgsTupel>::getArgNames(namevec);
			}
		};

		// 构造函数实参tuple，模版递归终止条件
		template <int ArgCount, typename ArgsTupel>
		struct TupleConstructor<0, ArgCount, ArgsTupel>
		{
			// 通过ios输入构造函数实参
			static int construct(ArgsTupel& args, std::iostream& ios) {
				return 0;
			}
			// 解析函数形参类型字符串数组
			static void getArgNames(std::vector<std::string>& namevec) {
				return;
			}
		};
	};

	//将类的非静态成员函数转化为std::function形式，方便处理
	template<typename T,typename R, typename ...Args>
	std::function<R(Args...)> classFuncToStdFunc(T* PtrThis,R(T::*func)(Args...)) {
		auto lmda = [PtrThis, func](Args... args) -> R{ //注意可变实参args的声明与使用
			return std::bind(func, PtrThis, args...)();
		};
		return std::function<R(Args...)>(lmda);
	}

	// 通过标准库输入调用给的函数
	template<typename R, typename ...Args>
	int callFuncByInput(std::function<R(Args...)>& func, std::iostream& ios) {
		//此处去除参数中的const与reference，防止tuple默认构造失败
		typedef function_traits<std::function<R(std::remove_const_t<std::remove_reference_t<Args>>...)>> ftraits;
		ftraits::argTuple_t args;
		int ret = ftraits::TupleConstructor<ftraits::nargs, ftraits::nargs, ftraits::argTuple_t>::construct(args, ios);
		if (ret) return -1;
		//#TODO2 现在命令行自动调用不会检查函数是否失败，只检查输入是否正确
		invoke(func, args);
		return 0;
	}

	// 通过标准库输入调用给的函数
	template<typename R, typename ...Args>
	std::vector<std::string> getFuncArgTypeNames(std::function<R(Args...)>& func) {
		//此处去除参数中的const与reference，防止tuple默认构造失败
		typedef function_traits<std::function<R(std::remove_const_t<std::remove_reference_t<Args>>...)>> ftraits;
		std::vector<std::string> argTypeNames;
		ftraits::TupleConstructor<ftraits::nargs, ftraits::nargs, ftraits::argTuple_t>::getArgNames(argTypeNames);
		return argTypeNames;
	}
}