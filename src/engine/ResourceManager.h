#pragma once
#include <unordered_map>
#include <string>
#include <iostream>
#include <assert.h>

#define QUOTE(X) #X

/// <summary>
/// the template of resource manager class. Each resource must have:
/// - a name to identify with
/// - probably a base folder
/// the resource manager also could use a different loader function
/// </summary>
/// <typeparam name="Resource"></typeparam>
using namespace std;

template <typename Resource>
class ResourceManager {
public:
	typedef Resource* (* PFNLOADER)(const char* name, void* pdata);

	// this is for file resource
	ResourceManager(PFNLOADER defaultLoader, const string& folder = "", Resource* defaultRes = nullptr, void* loaderData = (void*)0) {
		this->baseFolder = folder;
		this->loaderFn = defaultLoader;
		this->loaderData = loaderData;
		this->nullResource = defaultRes;

		assert(defaultLoader != nullptr);

		// if no default res, make our own
		/*if (!this->nullResource) {
			this->nullResource = new Resource();
		}*/

		currentLoader = loaderFn;

		this->counter = 0;
	}
	virtual ~ResourceManager() {
		for (auto ptrToRes : resources) {
			delete ptrToRes.second;
		}
		delete nullResource;
	}

	ResourceManager* withLoader(PFNLOADER fn) const {
		currentLoader = fn;
		return this;
	}
	Resource* load(const char* name) {
		// load and assign a name on it
		string fqName = baseFolder + "/" + string(name);
		// load a resource using the current loader function
		assert( (this->currentLoader != nullptr) && "No Loader for this resource!");
		Resource* res = this->currentLoader(fqName.c_str(), loaderData);
		assert(res != nullptr);
#ifdef _DEBUG
		cout << "Loading resource(" << name << " as " << fqName << "), got " << hex << res << dec << endl;
#endif
		// put it in the container, and set its id
		res->id = ++counter;
		resources.insert(make_pair(string(name), res));
		//resources[string(name)] = res;

		// reset default loader
		currentLoader = loaderFn;

		// return it
		return res;
	}

	/// <summary>
	/// just return what's loaded
	/// </summary>
	/// <param name="name"></param>
	/// <returns></returns>
	Resource* get(const char* name, bool autoload = true) {
		// try to look by name
		//Resource* r = resources[string(name)];
		auto it = resources.find(string(name));

		if (it == resources.end() && autoload) {
			cout << "Resource not found: " << name << ", attempting to load..." << endl;
			// welp, not found and no autoload
			Resource* r = this->load(name);

			assert(r && "Failed to load resource!");
			// return default resource
			// return nullResource;
			//return this->get(name, false);
			return r;
		}
		else if (!autoload) {
			return nullptr;
		}
		
		return it->second;
	}

	Resource* getRandom() {
		auto it = resources.begin();
		advance(it, rand() % resources.size());
		return it->second;
	}

	/// <summary>
	/// This put the resource in the manager, with an assigned name
	/// WARNING: WILL REPLACE WHAT'S IN THERE IF THERE IS A NAME CLASH
	/// </summary>
	/// <param name="name"></param>
	/// <param name="r"></param>
	/// <returns></returns>
	Resource* put(const char* name, Resource* r) {
		// make sure we don't put garbage
		assert(r != nullptr);

		// just insert it and set its id
		r->id = ++counter;
		resources.insert(make_pair(string(name), r));

		return r;
	}

	/// <summary>
	/// This will remove resource by value
	/// </summary>
	/// <param name="r"></param>
	bool remove(Resource* r, bool doDeletion = true) {
		auto it = resources.begin();
		while (it != resources.end()) {
			if (it.second == r) {
				delete it->second;
				resources.erase(it);
				return true;
			}
			it++;
		}
		return false;
	}

	/// <summary>
	/// remove by key
	/// </summary>
	/// <param name="name"></param>
	/// <returns></returns>
	bool remove(const char* name) {
		auto it = resources.find(string(name));
		if (it != resources.end()) {
			delete it->second;
			resources.erase(it);
			return true;
		}
		return false;
	}

	auto& getResourceMap() {
		return resources;
	}

	/// <summary>
	/// just return how much we stored now
	/// </summary>
	/// <returns></returns>
	size_t size() {
		return resources.size();
	}

	/// <summary>
	/// print debug contents
	/// </summary>
	void printDebug() {
		cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
		cout << "DEBUG_RESOURCE_CONTENTS: (" << resources.size() << ") ITEMS" << endl;
		int cnt = 0;
		for (pair<string, Resource*> e : resources) {
			cout << e.second->type() << "[" << ++cnt << "] : name(" << e.first << ") @ " << hex << e.second << dec << " id(" << e.second->id << ")" << endl;
		}
		cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
	}
protected:
	string baseFolder;
	PFNLOADER loaderFn, currentLoader;
	void* loaderData;
	unordered_map<string, Resource*> resources;

	// a null resource to avoid error?
	Resource* nullResource;

	// resource guid (in this manager scope)
	int counter;
};