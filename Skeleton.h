#pragma once

#include "skse_libray_facilities.h"
#include "std_library_facilities.h"

#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
#include "Addresses.h"
#endif

namespace Skeleton {

	class Skeleton {

	public:

		Skeleton(const Skeleton&) = delete;
		Skeleton(Skeleton&&) = delete;

		Skeleton& operator=(const Skeleton&) = delete;
		Skeleton& operator=(Skeleton&&) = delete;

		Skeleton() {}
		~Skeleton() {}

		bool HasExtraData(const std::string& filename, const std::string& node);
		bool HasExtraData(UInt32 id, const std::string& node);
		bool HasExtraData(NiAVObject* obj, const std::string& name);

	private:

		template<typename Func = std::function<bool(NiObject *)>>
		void Visit(const std::string&, Func);

		template<typename Func = std::function<bool(NiAVObject*)>>
		void Visit(UInt32, Func);
	};
}
