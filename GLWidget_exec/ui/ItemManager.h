#pragma once
#include "vrt.h"
#include "types.h"
class QTreeWidget;
class QTreeWidgetItem;
namespace vrt {
	class Primitive;

	class ItemManager{
	public:
		ItemManager(QTreeWidget* treeWgt);

		void addItem(std::shared_ptr<Primitive> prim);
		void deleteItem(std::shared_ptr<Primitive> prim);
		inline bool isExist(std::shared_ptr<Primitive> prim);

		struct Item
		{
			Item(std::shared_ptr<Primitive> pprim, QTreeWidgetItem* item) : prim(pprim), treeItem(item) {};
			std::shared_ptr<Primitive> prim;
			QTreeWidgetItem* treeItem;
			//QConnector
		};

	private:
		QTreeWidget* treeWgt_;

		std::map<int, Item> items_;
		std::vector<std::shared_ptr<Primitive>> selectedPrims;

	};

	inline bool ItemManager::isExist(std::shared_ptr<Primitive> prim)
	{
		return items_.find(prim->id()) != items_.end();
	}
}

