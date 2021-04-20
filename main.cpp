#include "common/IDebugLog.h"  // IDebugLog
#include "f4se_common/f4se_version.h"  // RUNTIME_VERSION
#include "f4se/PluginAPI.h"  // SKSEInterface, PluginInfo
#include "f4sE_common/Relocation.h"
#include "F4SE_common/SafeWrite.h"
#include "F4SE_common/BranchTrampoline.h"
#include "f4se/GameData.h"
#include "f4se/GameReferences.h"
#include "f4se/NiNodes.h"

#include <ShlObj.h>  // CSIDL_MYDOCUMENTS

#include "version.h"

static PluginHandle g_pluginHandle = kPluginHandle_Invalid;

static F4SEMessagingInterface* g_messaging = NULL;

// Here's offsets to hook into the main look of fallout4 vr
RelocAddr<uintptr_t> hookMainLoopFunc(0xd8187e);

typedef void(*_hookedMainLoop)();
RelocAddr<_hookedMainLoop> hookedMainLoop(0xd83ac0);



void printChildren(NiNode* child, std::string padding) {
	padding += "....";
	_MESSAGE("%s%s : children = %d", padding.c_str(), child->m_name.c_str(), child->m_children.m_emptyRunStart);

	if (child->GetAsNiNode())
	{
		for (auto i = 0; i < child->m_children.m_emptyRunStart; ++i) {
			auto nextNode = child->m_children.m_data[i] ? child->m_children.m_data[i]->GetAsNiNode() : nullptr;
			if (nextNode) {
				printChildren((NiNode*)nextNode, padding);
			}
		}
	}
}


void printNodes(NiNode* nde) {
	// print root node info first
	_MESSAGE("%s : children = %d", nde->m_name.c_str(), nde->m_children.m_emptyRunStart);

	std::string padding = "";

	for (auto i = 0; i < nde->m_children.m_emptyRunStart; ++i) {
		auto nextNode = nde->m_children.m_data[i] ? nde->m_children.m_data[i]->GetAsNiNode() : nullptr;
		if (nextNode) {
			printChildren((NiNode*)nextNode, padding);
		}
	}
}


void printSceneGraph() {

	// this checks if the player class is loaded
	if ((*g_player)->unkF0 && (*g_player)->unkF0->rootNode) {
		// Now let's find the top of the scene graph
		auto node = (*g_player)->unkF0->rootNode->m_parent->GetAsNiNode();    // Get the pattern of the player root node

		while (node->m_parent) {
			node = node->m_parent->GetAsNiNode();
		}
		
		// now print out scene graph
		printNodes(node);

		_MESSAGE("");
		_MESSAGE("Next frame");
		_MESSAGE("");
	}

	hookedMainLoop();
}

//Listener for F4SE Messages
void OnF4SEMessage(F4SEMessagingInterface::Message* msg)
{
	if (msg)
	{
		if (msg->type == F4SEMessagingInterface::kMessage_GameLoaded)
		{
			// PUT FUNCTIONS HERE THAT NEED TO RUN AFTER A NEW GAME OR SAVE GAME IS LOADED
		}
	}
}

extern "C" {
	bool F4SEPlugin_Query(const F4SEInterface* a_f4se, PluginInfo* a_info)
	{
		Sleep(5000);
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, R"(\\My Games\\Fallout4VR\\F4SE\\F4SE_print_scenegraph.log)");
		gLog.SetPrintLevel(IDebugLog::kLevel_DebugMessage);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

		_MESSAGE("F4SE Print Scene Graph  v%s", F4SEVRSCENEGRAPH_VERSION_VERSTRING);

		a_info->infoVersion = PluginInfo::kInfoVersion;
		a_info->name = "F4SEPrintScenegraph";
		a_info->version = F4SEVRSCENEGRAPH_VERSION_MAJOR;

		if (a_f4se->isEditor) {
			_FATALERROR("[FATAL ERROR] Loaded in editor, marking as incompatible!\n");
			return false;
		}

		a_f4se->runtimeVersion;
		if (a_f4se->runtimeVersion < RUNTIME_VR_VERSION_1_2_72)
		{
			_FATALERROR("Unsupported runtime version %s!\n", a_f4se->runtimeVersion);
			return false;
		}

		return true;
	}


	bool F4SEPlugin_Load(const F4SEInterface* a_f4se)
	{
		_MESSAGE("F4SE Scene Graph Init");

		g_pluginHandle = a_f4se->GetPluginHandle();

		if (g_pluginHandle == kPluginHandle_Invalid) {
			return false;
		}

		g_messaging = (F4SEMessagingInterface*)a_f4se->QueryInterface(kInterface_Messaging);
		g_messaging->RegisterListener(g_pluginHandle, "F4SE", OnF4SEMessage);

		if (!g_branchTrampoline.Create(1024 * 64))
		{
			_ERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
			return false;
		}

		// Usually put plugin functions here

		_MESSAGE("hooking main loop function");
		g_branchTrampoline.Write5Call(hookMainLoopFunc.GetUIntPtr(), (uintptr_t)printSceneGraph);
		_MESSAGE("successfully hooked main loop");

		_MESSAGE("F4SEPrintScenegraph Loaded");

		return true;
	}
};
