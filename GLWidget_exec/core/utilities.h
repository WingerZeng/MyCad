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
	// ��������ת�ַ���
	template <typename T>

	// ͨ��ʵ��tuple���ø�������
	template<typename Func, typename Tup>
	decltype(auto) invoke(Func&& func, Tup&& t)
	{
		constexpr auto size = std::tuple_size<typename std::decay<Tup>::type>::value;
		// ������������size��С��ģ������
		return invoke_impl(std::forward<Func>(func), std::forward<Tup>(t), std::make_index_sequence<size>{});
	}

	template<typename Func, typename Tup, std::size_t... Index>
	decltype(auto) invoke_impl(Func&& func, Tup&& t, std::index_sequence<Index...>)
	{
		// IndexΪ֮ǰ�����size��Сģ������0,1,2,3�������չ�󼴳�Ϊsize������
		return func(std::get<Index>(std::forward<Tup>(t))...);
	}

	//��Ҫ�Ƚ���ģ�����������ܽ���������ػ�
	template<typename T>
	struct function_traits {};

	// ��ȡ������Ϣ
	template<typename R, typename ...Args>
	struct function_traits<std::function<R(Args...)>>
	{
		function_traits() = delete;
		static const size_t nargs = sizeof...(Args); // �β�����

		typedef R result_type; // ��������
		typedef std::tuple<Args...> argTuple_t; //�β�tuple����

		// �����������,iΪ��0��ʼ�Ĳ�����������
		template <size_t i>
		struct arg
		{
			typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
		};

		// ���캯��ʵ��tuple
		template <int CountLast, int ArgCount, typename ArgsTupel>
		struct TupleConstructor
		{
			typedef typename std::tuple_element<ArgCount - CountLast, ArgsTupel>::type argtype;
			// ͨ��ios���빹�캯��ʵ��
			static int construct(ArgsTupel& args, std::iostream& ios) {
				argtype aArg;
				if (!(ios >> aArg)) return -1;
				std::get<ArgCount - CountLast>(args) = aArg;
				return TupleConstructor<CountLast - 1, ArgCount, ArgsTupel>::construct(args, ios);
			}
			// ���������β������ַ�������
			static void getArgNames(std::vector<std::string>& namevec) {
				namevec.push_back(TypeName<argtype>::Get());
				return TupleConstructor<CountLast - 1, ArgCount, ArgsTupel>::getArgNames(namevec);
			}
		};

		// ���캯��ʵ��tuple��ģ��ݹ���ֹ����
		template <int ArgCount, typename ArgsTupel>
		struct TupleConstructor<0, ArgCount, ArgsTupel>
		{
			// ͨ��ios���빹�캯��ʵ��
			static int construct(ArgsTupel& args, std::iostream& ios) {
				return 0;
			}
			// ���������β������ַ�������
			static void getArgNames(std::vector<std::string>& namevec) {
				return;
			}
		};
	};

	//����ķǾ�̬��Ա����ת��Ϊstd::function��ʽ�����㴦��
	template<typename T,typename R, typename ...Args>
	std::function<R(Args...)> classFuncToStdFunc(T* PtrThis,R(T::*func)(Args...)) {
		auto lmda = [PtrThis, func](Args... args) -> R{ //ע��ɱ�ʵ��args��������ʹ��
			return std::bind(func, PtrThis, args...)();
		};
		return std::function<R(Args...)>(lmda);
	}

	// ͨ����׼��������ø��ĺ���
	template<typename R, typename ...Args>
	int callFuncByInput(std::function<R(Args...)>& func, std::iostream& ios) {
		//�˴�ȥ�������е�const��reference����ֹtupleĬ�Ϲ���ʧ��
		typedef function_traits<std::function<R(std::remove_const_t<std::remove_reference_t<Args>>...)>> ftraits;
		ftraits::argTuple_t args;
		int ret = ftraits::TupleConstructor<ftraits::nargs, ftraits::nargs, ftraits::argTuple_t>::construct(args, ios);
		if (ret) return -1;
		invoke(func, args);
		return 0;
	}
	// ͨ����׼��������ø��ĺ���
	template<typename R, typename ...Args>
	std::vector<std::string> getFuncArgTypeNames(std::function<R(Args...)>& func) {
		//�˴�ȥ�������е�const��reference����ֹtupleĬ�Ϲ���ʧ��
		typedef function_traits<std::function<R(std::remove_const_t<std::remove_reference_t<Args>>...)>> ftraits;
		std::vector<std::string> argTypeNames;
		ftraits::TupleConstructor<ftraits::nargs, ftraits::nargs, ftraits::argTuple_t>::getArgNames(argTypeNames);
		return argTypeNames;
	}
}