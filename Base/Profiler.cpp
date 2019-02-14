#include "Profiler.h"

#include "glew\glew.h"

#include <cstdio>

Profiler::Profiler()
{
	currentQuery = 0;
	lastQuery = 0;
	queryBackBuffer = 0;
	queryFrontBuffer = 1;
	totalGPUTime = 0.0f;
}

void Profiler::Init()
{
	glCreateQueries(GL_TIME_ELAPSED, MAX_QUERIES, queryIDs[queryBackBuffer]);
	glCreateQueries(GL_TIME_ELAPSED, MAX_QUERIES, queryIDs[queryFrontBuffer]);
}

void Profiler::BeginQuery(const char *name)
{
	if (currentQuery >= MAX_QUERIES)
		return;

	glBeginQuery(GL_TIME_ELAPSED, queryIDs[queryBackBuffer][currentQuery]);

	queries[currentQuery].name = name;

	currentQuery++;
}

void Profiler::EndQuery()
{
	if ((currentQuery - lastQuery) == 1)
	{
		glEndQuery(GL_TIME_ELAPSED);
		lastQuery++;
	}
}

void Profiler::EndFrame()
{
	currentQuery = 0;
	lastQuery = 0;

	if (queryBackBuffer == 1)
	{
		queryBackBuffer = 0;
		queryFrontBuffer = 1;
	}
	else
	{
		queryBackBuffer = 1;
		queryFrontBuffer = 0;
	}
}

const char *Profiler::GetResults()
{
	const unsigned int MAX_CHARS = 1023;
	static char buffer[MAX_CHARS + 1];

	int charsWritten = 0;
	unsigned int bufferCount = MAX_CHARS;

	totalGPUTime = 0.0f;

	for (size_t i = 0; i < currentQuery; i++)
	{
		unsigned int value = 0;
		glGetQueryObjectuiv(queryIDs[queryFrontBuffer][i], GL_QUERY_RESULT, &value);
		float time = value / 1000000.0f;

		totalGPUTime += time;

		charsWritten += sprintf_s(buffer + charsWritten, bufferCount, "%s: %.3fms\n", queries[i].name, time);
		bufferCount -= charsWritten;
	}

	charsWritten += sprintf_s(buffer + charsWritten, bufferCount, "Total GPU time: %.3fms\n", totalGPUTime);

	//glGetQueryObjectuiv(queryIDs[queryFrontBuffer][id], GL_QUERY_RESULT, result);
	//glGetQueryObjectui64v(queryIDs[queryFrontBuffer][id], GL_QUERY_RESULT, result);
	return buffer;
}

void Profiler::Dispose()
{
	glDeleteQueries(MAX_QUERIES, queryIDs[queryBackBuffer]);
	glDeleteQueries(MAX_QUERIES, queryIDs[queryFrontBuffer]);
}
