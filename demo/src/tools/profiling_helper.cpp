// Project includes
#include "tools/profiling_helper.h"

// Project includes
#include "graphics/dx12_backend.h"

// System includes
#include <algorithm>

ProfilingHelper::ProfilingHelper()
{
}

ProfilingHelper::~ProfilingHelper()
{
}

void ProfilingHelper::initialize(GraphicsDevice device, CommandQueue cmdQ, uint32_t numScopes)
{
	m_NumScopes = numScopes;
	m_Scopes.resize(m_NumScopes);
	m_LastDurationArray.resize(m_NumScopes);
	m_MaxDurationArray.resize(m_NumScopes);
	for (uint32_t scopeIdx = 0; scopeIdx < m_NumScopes; ++scopeIdx)
	{
		m_Scopes[scopeIdx] = d3d12::profiling_scope::create_profiling_scope(device);
	}
}

void ProfilingHelper::release()
{
	for (uint32_t scopeIdx = 0; scopeIdx < m_NumScopes; ++scopeIdx)
		d3d12::profiling_scope::destroy_profiling_scope(m_Scopes[scopeIdx]);
}

void ProfilingHelper::start_profiling(CommandBuffer cmd, uint32_t index)
{
	d3d12::command_buffer::enable_profiling_scope(cmd, m_Scopes[index]);
}

void ProfilingHelper::end_profiling(CommandBuffer cmd, uint32_t index)
{
	d3d12::command_buffer::disable_profiling_scope(cmd, m_Scopes[index]);

}

void ProfilingHelper::process_scopes(CommandQueue cmdQ)
{
	for (uint32_t scopeIdx = 0; scopeIdx < m_NumScopes; ++scopeIdx)
	{
		uint64_t updateDuration = d3d12::profiling_scope::get_duration_us(m_Scopes[scopeIdx], cmdQ);
		m_MaxDurationArray[scopeIdx] = std::max(m_MaxDurationArray[scopeIdx], updateDuration);
		m_LastDurationArray[scopeIdx] = updateDuration;
	}
}

uint64_t ProfilingHelper::get_scope_last_duration(uint32_t index)
{
	return m_LastDurationArray[index];
}

uint64_t ProfilingHelper::get_scope_max_duration(uint32_t index)
{
	return m_MaxDurationArray[index];
}

void ProfilingHelper::reset_durations()
{
	for (uint32_t scopeIdx = 0; scopeIdx < m_NumScopes; ++scopeIdx)
	{
		m_MaxDurationArray[scopeIdx] = 0;
		m_LastDurationArray[scopeIdx] = 0;
	}
}
