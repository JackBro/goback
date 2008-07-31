#include "ModuleManager.h"

// TODO: Find a better way to link the modules without including them here
#include "DataFormatElf.h"
#include "WorkModeDisasm.h"
#include "WorkModeHex.h"

ModuleManager *ModuleManager::_instance = NULL;

ModuleManager *ModuleManager::getInstance() {
	if (!_instance) {
		_instance = new ModuleManager();
	}

	return _instance;
}

ModuleManager::ModuleManager() {
	// Populate the lists of modules
	// TODO: This could be used to load plugins
	_dataFormats.push_back((DataFormatModule *)new DataFormatElfModule());
	_workModes.push_back((WorkModeModule *)new WorkModeDisasmModule());
	_workModes.push_back((WorkModeModule *)new WorkModeHexModule());
}

ModuleManager::~ModuleManager() {
	// Delete the DataFormatModules
	DataFormatModules::iterator df = _dataFormats.begin();
	while (df != _dataFormats.end()) {
		// Delete it
		delete *df;
		df++;
	}

	// Delete the WorkModeModules
	WorkModeModules::iterator wm = _workModes.begin();
	while (wm != _workModes.end()) {
		// Delete it
		delete *wm;
		wm++;
	}
}

DataFormatModules ModuleManager::getDataFormats() const {
	return _dataFormats;
}

WorkModeModules ModuleManager::getWorkModes() const {
	return _workModes;
}
