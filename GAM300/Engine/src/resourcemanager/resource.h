///Resource.h
///ibai.abaunza 
///Contains interface classes for the resources
#pragma once

#include <string>
#include <iostream>

typedef struct stat FileStatus;

///Base resource class
class IResource
{
public:
	IResource() : mLastModifiedT(0),mStatus() {};
	virtual ~IResource() {};
	FileStatus getModifiedT();
	long long currentModT();
	void setModifiedT();
	void setName(std::string _Name);
	std::string getName() const;
private:

protected:
	long long mLastModifiedT; //last modified time

	FileStatus mStatus{};
	std::string mName{};
	std::string mExtension{};
};


///Interface class that will be on the map of the resourcemanager
template<typename T>
class TResource: public IResource
{
public:

	//returns actual texture etc.
	T* get() { return mActualResource; }
	void SetResource(T* _resource) { mActualResource = _resource; }
	void Release() { if (mActualResource) delete mActualResource; }
	~TResource() { Release(); }

private:
	T* mActualResource = nullptr;
};

//just an interface, each department should make its own importer loading properly the resource
class IresourceImporter 
{
public:
	// Should be overriden. Each person should chose between just returning a tresource (with new) 
	// or making its own resource clase deriving from IResource
	virtual std::shared_ptr<IResource> load(const char* filename, bool reload = false) = 0;
};



/************************************* Importer Example *************************************/

class Dummy
{
public:
	void print() {  }
};

class DummyImporter : public IresourceImporter
{
public:
	virtual std::shared_ptr<IResource> load(const char* filename, bool reload = false) {
		std::shared_ptr<TResource<Dummy>> res= std::make_shared<TResource<Dummy>>();
		return res;
	}
};




/************************************** Actual Importers **************************************/
class TextureImporter : public IresourceImporter
{
public:
	std::shared_ptr<IResource> load(const char* filename, bool reload = false) override;
	/* unload????? */
};

class CubemapImporter : public IresourceImporter
{
public:
	std::shared_ptr<IResource> load(const char* filename, bool reload = false) override;
	/* unload????? */
};

class ModelImporter : public IresourceImporter
{
public:
	std::shared_ptr<IResource> load(const char* filename, bool reload = false) override;
	/* unload????? */
};

class FolderImporter : public IresourceImporter
{
public:
	std::shared_ptr<IResource> load(const char* filename, bool reload = false) override;
	/* unload????? */
};

class FontImporter : public IresourceImporter
{
public:
	std::shared_ptr<IResource> load(const char* filename, bool reload = false) override;
	/* unload????? */
};