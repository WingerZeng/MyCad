#include "ItemManager.h"
#include "Primitive.h"
#include "MainWindow.h"
#include "glrender/Scene.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>
namespace vrt{

	ItemManager::ItemManager(QTreeWidget* treeWgt) :treeWgt_(treeWgt)
	{
		treeWgt_->connect(treeWgt_, &QTreeWidget::itemSelectionChanged, [this]() {
			auto list = this->treeWgt_->selectedItems();

			for (const auto& prim : selectedPrims) {
				prim->setSelected(false);
			}
			selectedPrims.clear();
			for (const auto& item : list) {
				items_[item->text(1).toInt()].prim->setSelected(true);
				//#PERF5 每次都要重新清空选择项
				selectedPrims.push_back(items_[item->text(1).toInt()].prim);
			}

			});
	}

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

		//#TODO 删除子节点
	}
}