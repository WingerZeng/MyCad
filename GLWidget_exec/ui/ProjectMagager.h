#pragma once
#include "vrt.h"
#include "types.h"
#include "Singleton.h"

#define  PROPTR  (Singleton<ProjectMagager>::getSingleton())
namespace vrt {
	class ProjectMagager: Singleton<ProjectMagager>
	{
	public:
		friend Singleton<ProjectMagager>;
		~ProjectMagager();

		QString swPath() const { return SoftwarePath(); }
		QString SoftwarePath() const { return softwarePath_; }
		void setSoftwarePath(QString val) { softwarePath_ = val; }
	private:
		ProjectMagager();

		QString softwarePath_;
	};
}

