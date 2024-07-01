#pragma once

// Project includes
#include "graphics/types.h"

// Sytem includes
#include <vector>

class ProfilingHelper
{
public:
	// Cst & Dst
	ProfilingHelper();
	~ProfilingHelper();

	// Initialization and release
	void initialize(GraphicsDevice device, CommandQueue cmdQ, uint32_t numScopes);
	void release();

	// Runtime functions
	void start_profiling(CommandBuffer cmd, uint32_t index);
	void end_profiling(CommandBuffer cmd, uint32_t index);
	void process_scopes(CommandQueue cmdQ);
	uint64_t get_scope_last_duration(uint32_t index);
	uint64_t get_scope_max_duration(uint32_t index);
	void reset_durations();

private:
	uint32_t m_NumScopes = 0;
	std::vector<ProfilingScope> m_Scopes;
	std::vector<uint64_t> m_LastDurationArray;
	std::vector<uint64_t> m_MaxDurationArray;
};