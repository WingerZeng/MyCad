#pragma once
#include "vrt.h"
#include "types.h"
#include <QMainWindow>
#include <QTreeWidget>
#include "Singleton.h"
#include "CadInterface.h"
#define MAIPTR (Singleton<MainWindow>::getSingleton())

namespace Ui {
	class MainWindow;
}

class QTreeWidget;

namespace vrt {
	class CadInterface;
	class ScriptList;
	class ItemManager;
	class Console;
	class MainWindow: public QMainWindow, private Singleton<MainWindow>
	{
		Q_OBJECT
		friend Singleton<MainWindow>;
	public:
		~MainWindow();
		vrt::CadInterface* CadApi() const { return cadapi; }
		Scene* getScene() const;
		QTreeWidget* getCadTreeWidget() const;
		Console* getConsole();
		void cadTreeUnselectedAll();

		vrt::ScriptList* scplist() const { return ui_scplist; }
		std::shared_ptr<vrt::ItemManager> ItemMng() const { return itemMng_; }
	private:
		MainWindow(QWidget* parent = nullptr);
		void setupScripts();

	public slots:
		//void updateCadElements(CadInterface::ELEMEMNT_TYPE elemtype, CadInterface::ElemHandle elem);
		void cleanAllSelection();

	private:
		CadInterface* cadapi;

		std::map<CadInterface::ElemHandle, QTreeWidgetItem*> cadElemMap;

		Ui::MainWindow* ui;

		ScriptList* ui_scplist;

		std::shared_ptr<ItemManager> itemMng_;
	};
}

