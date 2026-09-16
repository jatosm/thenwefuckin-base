#include "combat.h"
#include "../../variables/variables.h"
#include "../../globals/globals.h"
#include "../../../memory/memory.h"
#include "../../../sdk/offsets.h"
#include "../../../sdk/sdk.h"

#include <Windows.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Combat {
namespace {








static std::vector<std::uintptr_t> g_cdAddrs;


const char* kCooldownKeys[] = {
	"ShootingCooldown", "Cooldown", "cooldown", "ToleranceCooldown",
};

void ZeroTool(RBX::RbxInstance tool, double v)
{
	static std::unordered_set<std::uintptr_t> logged;
	for (auto& c : tool.GetChildList())
	{
		if (c.GetClass() != "NumberValue")
			continue;
		const std::string n = c.GetName();
		bool match = false;
		for (auto k : kCooldownKeys)
		{
			if (n == k)
			{
				match = true;
				break;
			}
		}
		if (!match)
			continue;
		g_cdAddrs.push_back(c.Addr);
		const double cur = memory->read<double>(c.Addr + Offsets::Misc::Value);
		if (cur != v)
		{
			if (logged.insert(c.Addr).second)
				printf("[combat] pin %s @0x%llX tool=%s\n", n.c_str(),
					(unsigned long long)c.Addr, tool.GetName().c_str());
			memory->write<double>(c.Addr + Offsets::Misc::Value, v);
		}
	}
}

void TickEnforce(double v)
{
	for (auto a : g_cdAddrs)
	{
		double cur = 0;
		if (!memory->read_raw(a + Offsets::Misc::Value, (std::uint8_t*)&cur, 8))
			continue; 
		if (cur == v)
			continue;
		if (cur < 0.0 || cur > 100.0)
			continue; 
		memory->write<double>(a + Offsets::Misc::Value, v);
	}
}

void DumpToolValues(RBX::RbxInstance tool)
{
	printf("[combat] holding %s\n", tool.GetName().c_str());
	for (auto& c : tool.GetChildList())
	{
		const std::string cls = c.GetClass();
		if (cls == "NumberValue")
			printf("[combat]   %s (Number) = %f\n", c.GetName().c_str(),
				memory->read<double>(c.Addr + Offsets::Misc::Value));
		else if (cls == "IntValue")
			printf("[combat]   %s (Int) = %d\n", c.GetName().c_str(),
				memory->read<int>(c.Addr + Offsets::Misc::Value));
	}
}

void TickRapidFire()
{
	static std::string lastTool;
	static std::uintptr_t lastLp = 0;
	if (!variables::Aimbot::rapidFire)
	{
		lastTool.clear();
		g_cdAddrs.clear();
		return;
	}
	if (!Globals::localPlayer.Addr)
		return;
	if (Globals::localPlayer.Addr != lastLp)
	{
		lastLp = Globals::localPlayer.Addr;
		g_cdAddrs.clear();
		lastTool.clear();
	}
	g_cdAddrs.clear();
	const double v = (double)variables::Aimbot::rapidFireValue;
	RBX::RbxInstance bp = Globals::localPlayer.FindChild("Backpack");
	if (bp)
	{
		for (auto& t : bp.GetChildList())
		{
			if (t.GetClass() == "Tool")
				ZeroTool(t, v);
		}
	}
	RBX::RbxInstance ch = Globals::localPlayer.GetModelRef();
	if (ch)
	{
		RBX::RbxInstance equipped;
		for (auto& t : ch.GetChildList())
		{
			if (t.GetClass() != "Tool")
				continue;
			if (!equipped)
				equipped = t;
			ZeroTool(t, v);
		}
		const std::string curTool = equipped ? equipped.GetName() : "";
		if (curTool != lastTool)
		{
			lastTool = curTool;
			variables::Aimbot::detected_weapon_name =
				curTool.empty() ? "None" : curTool;
			if (equipped)
				DumpToolValues(equipped);
		}
	}
}

} 

void Loop()
{
	using namespace std::chrono_literals;
	ULONGLONG lastDiscover = 0;
	while (Globals::running)
	{
		if (memory->IsConnected() && Globals::localPlayer.Addr &&
			variables::Aimbot::rapidFire)
		{
			TickEnforce((double)variables::Aimbot::rapidFireValue);
			const ULONGLONG now = GetTickCount64();
			if (now - lastDiscover >= 250)
			{
				lastDiscover = now;
				TickRapidFire();
			}
		}
		std::this_thread::sleep_for(1ms);
	}
}

} 
