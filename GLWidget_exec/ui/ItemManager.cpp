#include "ItemManager.h"
#include "Primitive.h"
#include "MainWindow.h"
#include "glrender/Scene.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>
namespace vrt{

	void ItemManager::addItem(std::shared_ptr<Primitive> prim)
	{
		if (isExist(prim))
			return;

		QTreeWidgetItem* newItem = new QTreeWidgetItem({ prim->name() ,QString::number(prim->id())});
		treeWgt_->addTopLevelItem(newItem);
		items_[prim->id()] = Item(prim, newItem);

		MAIPTR->getScene()->addPrimitive(prim);
	}

	void ItemManager::deleteItem(std::shared_ptr<Primitive> prim)
	{
		if (!isExist(prim))
			return;

		treeWgt_->takeTopLevelItem(treeWgt_->indexOfTopLevelItem(items_[prim->id()].treeItem));
		MAIPTR->getScene()->delPrimitive(items_[prim->id()].prim->id());

		//#TODO É¾³ý×Ó½Úµã
	}

	


}