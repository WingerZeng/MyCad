#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "ItemManager.h"
#include "Scripts.h"
namespace vrt{
	
	MainWindow::MainWindow(QWidget* parent)
		:QMainWindow(parent),ui(new Ui::MainWindow)
	{
		ui->setupUi(this);
		cadapi = new CadInterface();
		connect(cadapi, SIGNAL(elementChanged(ELEMEMNT_TYPE, ElemHandle)), this, SLOT(cleanAllSelection()));
		connect(getCadTreeWidget(), SIGNAL(itemSelectionChanged()), cadapi, SLOT(itemSelectionChanged()));

		ui_scplist = new ScriptList;
		connect(ui->actionScript_List, &QAction::triggered, [this]() { this->scplist()->show(); });

		setupScripts();

		itemMng_.reset(new ItemManager(ui->treeWidget_general));
	}

	void MainWindow::setupScripts()
	{
		scplist()->init();
	}

	void MainWindow::cleanAllSelection()
	{
		getCadTreeWidget()->clearSelection();
	}

	MainWindow::~MainWindow()
	{
		//ui_scplist->saveAll();
		delete ui;
	}

	vrt::Scene* MainWindow::getScene() const //void Cadapi(vrt::CadInterface* val)
	{
		return ui->openGLWidget;
	}

	QTreeWidget* MainWindow::getCadTreeWidget() const
	{
		return ui->treeWidget;
	}

	vrt::Console* MainWindow::getConsole()
	{
		return ui->textEdit;
	}

	void MainWindow::cadTreeUnselectedAll()
	{
		auto list = getCadTreeWidget()->selectedItems();
		for (const auto& item:list)
		{
			item->setSelected(false);
		}
	}

}