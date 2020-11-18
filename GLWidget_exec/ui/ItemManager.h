#pragma once
#include "vrt.h"
#include "types.h"
#include "Primitive.h"
class QTreeWidget;
class QTreeWidgetItem;
namespace vrt {
	class Primitive;

	class ItemManager{
	public:
		ItemManager(QTreeWidget* treeWgt);

		void addItem(std::shared_ptr<Primitive> prim);
		void addItem(std::vector<std::shared_ptr<Primitive>> prims);
		std::shared_ptr<Primitive> getItem(int id);

		template<class T>
		void getItem(int id, std::shared_ptr<T>& prim);

		void delItem(std::shared_ptr<Primitive> prim);
		inline bool isExist(std::shared_ptr<Primitive> prim);

		int clearSelected();

		struct Item
		{
			Item(std::shared_ptr<Primitive> pprim, QTreeWidgetItem* item) : prim(pprim), treeItem(item) {};
			Item() = default;
			std::shared_ptr<Primitive> prim;
			QTreeWidgetItem* treeItem;
			//QConnector
		};

	private:
		QTreeWidget* treeWgt_;

		std::map<int, Item> items_;
		std::vector<std::shared_ptr<Primitive>> selectedPrims;

	};

	template<class T>
	void ItemManager::getItem(int id, std::shared_ptr<T>& prim)
	{
		if (items_.find(id) == items_.end())
		{
			prim = nullptr;
			return;
		}
		prim = std::dynamic_pointer_cast<T>(items_[id].prim);
	}

	inline bool ItemManager::isExist(std::shared_ptr<Primitive> prim)
	{
		return items_.find(prim->id()) != items_.end();
	}
}

