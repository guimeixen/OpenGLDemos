#pragma once

struct Query
{
	const char *name;
};

class Profiler
{
public:
	Profiler();

	void Init();
	void BeginQuery(const char *name);
	void EndQuery();
	void EndFrame();
	const char *GetResults();
	void Dispose();

private:
	static const unsigned int MAX_QUERIES = 32;

	unsigned int queryIDs[2][MAX_QUERIES];
	Query queries[MAX_QUERIES];
	float totalGPUTime;
	unsigned int currentQuery;
	unsigned int lastQuery;
	unsigned int queryBackBuffer;
	unsigned int queryFrontBuffer;
};

