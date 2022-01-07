#pragma once

/// <summary>
/// The base of all resource class
/// </summary>
class Resource {
public:
	Resource(): id(-1) {}
	virtual ~Resource() {}

	virtual const char* type() = 0;

	int id;	// a resource must have an id for easy stuffs, default to -1, will be set by manager
};