#include "Console.h"
#include <sstream>
#include "CadInterface.h"
#include "MainWindow.h"
#include "glrender/Scene.h"
#include "utilities.h"
#include "primitives/Subdivide.h"
#include "primitives/PPolygonMesh.h"

#define  BEGIN_FUNC_REGISTER\
	std::function<int(std::iostream& sin)> lambda;
	
//绑定静态函数，将函数调用作为lambda绑定到字典树中
#define REGISTER_CMD_FUNC(FuncName,Func) {\
	std::function<std::remove_pointer_t<decltype(Func)>> func(Func); \
	lambda = std::function<int(std::iostream& sin)>([](std::iostream& sin){\
		std::function<std::remove_pointer_t<decltype(Func)>> func(Func); \
		return callFuncByInput(func, sin); \
	});\
	triemap[#FuncName] = lambda; \
	funcArgNameList[#FuncName] = getFuncArgTypeNames(func);}  //生成函数形参字符串列表，用于生成帮助

//绑定类成员函数，将函数调用作为lambda绑定到字典树中
#define REGISTER_CLASS_CMD_FUNC(FuncName,ClassPtr,ClassName,Func) {\
	auto ptr = ClassPtr;\
	lambda = std::function<int(std::iostream& sin)>([ptr](std::iostream& sin){\
		return callFuncByInput(classFuncToStdFunc<ClassName>((ptr),(&ClassName::Func)), sin); \
	});\
	triemap[#FuncName] = lambda;\
	auto func = classFuncToStdFunc<ClassName>((ptr),(&ClassName::Func)); \
	funcArgNameList[#FuncName] = getFuncArgTypeNames(func);}   //生成函数形参字符串列表，用于生成帮助

namespace vrt{
	Console::Console(QWidget* parent)
		:QConsole(parent,welcomeText)
	{
		connect(this, &QConsole::execCommand, [this](const QString& cmd) { EXEPTR->exec(cmd, this); });
		setPrompt(">>");
		setOutColor(QColor(0.0,0.0,1.0));
		QFont font("Arial",9);
		setFont(font);
	}

	Console::~Console()
	{
	}

	const QString Console::welcomeText = "Welcome to MyCad!\nType \"help;\" command to get help.";

	QStringList Console::suggestCommand(const QString &cmd, QString &prefix)
	{
		return EXEPTR->suggestCommand(cmd, prefix);
	}

	bool Console::isCommandComplete(const QString &command)
	{
;		if (command.indexOf(";") == -1) return false;
		else return true;
	}

	QString Console::addCommandToHistory(const QString &command)
	{
		//Add the command to the recordedScript list
		recordedScript.append(command);
		//update the history and its index
		QString modifiedCommand = command;
		modifiedCommand.replace("\n", "\\n");
		modifiedCommand = modifiedCommand.left(modifiedCommand.indexOf(';') + 1);
		history.append(modifiedCommand);
		historyIndex = history.size();
		//emit the commandExecuted signal
		Q_EMIT commandAddedToHistory(modifiedCommand);
		return "";
	}

	void Executor::exec(const QString& command, Console* con)
	{
		std::vector<std::string> cmds;
		int idx;
		QString cmd = modifyCmd(command);
		//对于多行输入(脚本)，先分解为多个命令行
		while ((idx = cmd.indexOf(";")) != -1) {
			cmds.push_back(cmd.left(idx).toStdString());
			cmd = cmd.right(cmd.size() - idx - 1);
		}

		con_ = con;
		int ret = 0;
		for (const auto& cmd : cmds) {
			std::stringstream sin(cmd);
			std::string funcname;
			sin >> funcname;
			if (con) con->printCommandExecutionResults(QString::fromStdString(">Do " + funcname), QConsole::Partial);
			if (triemap.find(funcname) != triemap.end()) {
				//通过函数名调用字典树中预先绑定的函数
				ret = triemap[funcname](sin);
				if (ret) {
					break;
				}
			}
			else {
				ret = -2;
				break;
			}
		}

		if (con_) {
			if (ret) {
				if (ret == -2) {
					con->printCommandExecutionResults("Valid command!", QConsole::Error);
				}
				else 
					con->printCommandExecutionResults("Failed!", QConsole::Error);
			}
			else
				con->printCommandExecutionResults("Done.");
		}
		
		MAIPTR->getScene()->update();
	}

	int Executor::getHelp()
	{
		const QString msg(\
			"Please use commands to operate the software. Every command ends with a \';\'.		\n"
			"******All Commands******																		\n"
			"mvfs	[Coord]											;							\n"
			"mev	[VertexID]	[LoopID]	[Coord]					;							\n"
			"mef	[VertexID]	[VertexID]	[LoopID]				;							\n"
			"kfmrh	[FaceID]	[FaceID]							;							\n"
			"kemr	[LoopID]	[HalfEdgeID]						;							\n"
			"sweep	[SolidID]	[FaceID]	[Vector]	[Float]		;							\n"
			"createFaceFromLoop	[CoordLoops]						;							\n"
			"																					\n"
			"******Ui help******																			\n"
			"Press left mouse button to rotate, mid button to translate.						\n"
			"Use item list to determine the id of elements.										\n"
			"																					\n"
			"******Test sample (a cube with hole)******												\n"
			"createface {(1,1,0  -1,1,0  -1,-1,0  1,-1,0) (0.5,0.5,0  0.5,-0.5,0  -0.5,-0.5,0  0.3,0,0  -0.5,0.5,0)}; \n"
			"sweep  0  1 0,0,1 1;	\n"
			"																					\n"
			"******More******	\n"
			"For more more examples and details, please enter script list in the menu.   		\n"
		); //#TODO
		EXEPTR->printMessage(msg, QConsole::Partial);
		return 0;
	}

	int Executor::printMessage(const QString& msg, Console::ResultType type)
	{
		if (con_) {
			con_->printCommandExecutionResults(msg, type);
		}
		return 0;
	}

	QStringList Executor::suggestCommand(const QString &cmd, QString &prefix)
	{
		auto itpair = triemap.equal_prefix_range(cmd.toStdString());
		QStringList ret;
		for (auto it = itpair.first; it != itpair.second; it++) {
			ret << QString::fromStdString(it.key());
		}
		return ret;
	}

	Executor::Executor()
	{
		//注册命令行函数
		BEGIN_FUNC_REGISTER
			REGISTER_CMD_FUNC(help, Executor::getHelp)
			REGISTER_CLASS_CMD_FUNC(mvfs, MAIPTR->CadApi(), CadInterface, mvfs)
			REGISTER_CLASS_CMD_FUNC(mev, MAIPTR->CadApi(), CadInterface, mev)
			REGISTER_CLASS_CMD_FUNC(mef, MAIPTR->CadApi(), CadInterface, mef)
			REGISTER_CLASS_CMD_FUNC(kfmrh, MAIPTR->CadApi(), CadInterface, kfmrh)
			REGISTER_CLASS_CMD_FUNC(kemr, MAIPTR->CadApi(), CadInterface, kemr)
			REGISTER_CLASS_CMD_FUNC(sweep, MAIPTR->CadApi(), CadInterface, sweep)
			REGISTER_CLASS_CMD_FUNC(createface, MAIPTR->CadApi(), CadInterface, createFaceFromLoop)
			REGISTER_CLASS_CMD_FUNC(toPolygonMesh, MAIPTR->CadApi(), CadInterface, solidToPolygonMesh)
			REGISTER_CMD_FUNC(DooSabinPolygons, dooSabinSubdivPolygonMesh)
			REGISTER_CMD_FUNC(CatmullClarkPolygons, catmullClarkSubdivPolygonMesh)
			REGISTER_CMD_FUNC(DooSabinTriangles, dooSabinSubdivTriangles)
			REGISTER_CMD_FUNC(CatmullClarkTriangles, catmullClarkSubdivTriangles)
			REGISTER_CMD_FUNC(LoopTriangles, loopSubdivideTriangles)
			REGISTER_CMD_FUNC(triangulatePolygonMesh, triangulatePolygonMesh)
			REGISTER_CLASS_CMD_FUNC(wireframe, MAIPTR->getScene(), Scene, wireFrameMode)
	}

	QString Executor::modifyCmd(const QString& command)
	{
		QString modifiedCommand = command;
		modifiedCommand = modifiedCommand.left(modifiedCommand.lastIndexOf(';') + 1);
		return modifiedCommand;
	}
}