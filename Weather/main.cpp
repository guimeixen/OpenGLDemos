#include "Window.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "Quad.h"
#include "Shader.h"
#include "Utils.h"
#include "TextRenderer.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_opengl3.h"

#define CURL_STATICLIB
#include "curl/curl.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

struct RGBA
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

enum ParamIndices {
	RH_INDEX,
	TMP_INDEX,
	CAPE_INDEX,
	GUST_INDEX,
	DEWPOINT_INDEX,
	LCL_INDEX,
	//TSTORM_INDEX,
	NUM_PARAMS
};

size_t WriteMessage(void *contents, size_t size, size_t nmemb, std::string *userp)
{
	userp->append((char*)contents, size * nmemb);
	return size * nmemb;
}

size_t WriteData(void *contents, size_t size, size_t nmemb, std::ofstream *file)
{
	file->write((char*)contents, size * nmemb);
	return size * nmemb;
}

int DownloadGRIB()
{
	CURL *curl;
	curl = curl_easy_init();

	if (curl)
	{
		const std::string serverName = "grbsrv.opengribs.org";
		const std::string scriptPath = "/";
		const std::string phpFileName = scriptPath + "getmygribs2.php?";
		//const std::string ptype = "Unknown";

		std::string params = phpFileName + "osys=Unknown&ver=1.2.4&model=gfs_p25_&la1=36.75&la2=42.50&lo1=350.25&lo2=354.00&intv=3&days=3&cyc=last&par=T;H;c;G;&wmdl=none&wpar=";

		const std::string url = "http://" + serverName + params;

		std::cout << url << '\n';
		std::string response;
		//std::string response = "{\"status\":true,\"message\":{\"url\":\"http:\/\/grbsrv.opengribs.org\/downloads\/1553615155508\/20190326_154555_GFS_P25_.grb2\",\"size\":18607,\"sha1\":\"c5d1bfdb08020a309aff491d7f65eb2c0f4cc118\"}}";

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMessage);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		CURLcode res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			std::cout << "Error performing get request\n";
			return -1;
		}

		size_t idx1 = response.find("http");
		size_t idx2 = response.find("grb2");
		std::string gribUrl = response.substr(idx1, idx2 - idx1 + 4);		// +4 to include grb2

		gribUrl.erase(std::remove(gribUrl.begin(), gribUrl.end(), '\\'), gribUrl.end());

		std::cout << response << '\n';
		std::cout << gribUrl << '\n';

		std::ofstream file("latest.grb2", std::ios::binary);

		curl_easy_setopt(curl, CURLOPT_URL, gribUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			std::cout << "Error downloading grib\n";
			return -1;
		}

		curl_easy_cleanup(curl);
	}

	return 0;
}

int ConvertGRIBToText()
{
	if (std::system("wgrib2.exe latest.grb2 -spread latest.txt") != 0)
	{
		std::cout << "Failed to convert grib to text\n";
		return -1;
	}

	return 0;
}

glm::vec3 getTMPColor(float value)
{
	const int NUM_COLORS = 6;
	const static float color[NUM_COLORS][3] = { {1,0,1}, {0,0,1}, {0,1,0}, {1,1,0}, {1,0,0}, {1,0,1} };
	// A static array of 6 colors:  (pink, blue, green, yellow, red, pink) using {r,g,b} for each.

	int idx1;        // |-- Our desired color will be between these two indexes in "color".
	int idx2;        // |
	float fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

	if (value <= 0)			// accounts for an input <=0
	{
		idx1 = idx2 = 0;
	}
	else if (value >= 1)			 // accounts for an input >=0
	{
		idx1 = idx2 = NUM_COLORS - 1;
	}
	else
	{
		value = value * (NUM_COLORS - 1);				// Will multiply value by 5.
		idx1 = (int)std::floor(value);                  // Our desired color will be after this index.
		idx2 = idx1 + 1;								// ... and before this index (inclusive).
		fractBetween = value - float(idx1);				// Distance between the two indexes (0-1).
	}

	glm::vec3 rgb;
	rgb.x = (color[idx2][0] - color[idx1][0]) * fractBetween + color[idx1][0];
	rgb.y = (color[idx2][1] - color[idx1][1]) * fractBetween + color[idx1][1];
	rgb.z = (color[idx2][2] - color[idx1][2]) * fractBetween + color[idx1][2];

	return rgb;
}

glm::vec3 getRHColor(float value)
{
	const int NUM_COLORS = 3;
	const static float color[NUM_COLORS][3] = { {1,0,0}, {1,1,0}, {0,0,1} };

	int idx1;
	int idx2;
	float fractBetween = 0;

	if (value <= 0)
	{
		idx1 = idx2 = 0;
	}
	else if (value >= 1)
	{
		idx1 = idx2 = NUM_COLORS - 1;
	}
	else
	{
		value = value * (NUM_COLORS - 1);
		idx1 = (int)std::floor(value);
		idx2 = idx1 + 1;
		fractBetween = value - float(idx1);
	}

	glm::vec3 rgb;
	rgb.x = (color[idx2][0] - color[idx1][0]) * fractBetween + color[idx1][0];
	rgb.y = (color[idx2][1] - color[idx1][1]) * fractBetween + color[idx1][1];
	rgb.z = (color[idx2][2] - color[idx1][2]) * fractBetween + color[idx1][2];

	return rgb;
}

glm::vec3 getCAPEColor(float value)
{
	const int NUM_COLORS = 5;
	const static float color[NUM_COLORS][3] = { {0,0,1}, {0,1,0}, {1,1,0}, {1,0,0}, {1,0,1} };

	int idx1;
	int idx2;
	float fractBetween = 0;

	if (value <= 0)
	{
		idx1 = idx2 = 0;
	}
	else if (value >= 1)
	{
		idx1 = idx2 = NUM_COLORS - 1;
	}
	else
	{
		value = value * (NUM_COLORS - 1);
		idx1 = (int)std::floor(value);
		idx2 = idx1 + 1;
		fractBetween = value - float(idx1);
	}

	glm::vec3 rgb;
	rgb.x = (color[idx2][0] - color[idx1][0]) * fractBetween + color[idx1][0];
	rgb.y = (color[idx2][1] - color[idx1][1]) * fractBetween + color[idx1][1];
	rgb.z = (color[idx2][2] - color[idx1][2]) * fractBetween + color[idx1][2];

	return rgb;
}

glm::vec3 getWindColor(float value)
{
	const int NUM_COLORS = 5;
	const static float color[NUM_COLORS][3] = { {0,1,1}, {0,1,0}, {1,1,0}, {1,0,0}, {1,0,1} };

	int idx1;
	int idx2;
	float fractBetween = 0;

	if (value <= 0)
	{
		idx1 = idx2 = 0;
	}
	else if (value >= 1)
	{
		idx1 = idx2 = NUM_COLORS - 1;
	}
	else
	{
		value = value * (NUM_COLORS - 1);
		idx1 = (int)std::floor(value);
		idx2 = idx1 + 1;
		fractBetween = value - float(idx1);
	}

	glm::vec3 rgb;
	rgb.x = (color[idx2][0] - color[idx1][0]) * fractBetween + color[idx1][0];
	rgb.y = (color[idx2][1] - color[idx1][1]) * fractBetween + color[idx1][1];
	rgb.z = (color[idx2][2] - color[idx1][2]) * fractBetween + color[idx1][2];

	return rgb;
}

class Record {
public:
	void AddValue(int valueType, float value)
	{
		values[valueType].push_back(value);
	}

	void Finish(unsigned int texWidth, unsigned int texHeight)
	{
		width = texWidth;
		height = texHeight;

		std::vector<RGBA> colors[NUM_PARAMS];

		for (size_t i = 0; i < NUM_PARAMS; i++)
		{
			colors[i].resize(GetNumValues());
		}

		size_t numValues = GetNumValues();

		values[DEWPOINT_INDEX].resize(numValues);
		values[LCL_INDEX].resize(numValues);
		//values[TSTORM_INDEX].resize(numValues);

		for (size_t i = 0; i < numValues; i++)
		{
			glm::vec3 rgb = getRHColor(values[0][i] / 100.0f);
			colors[0][i] = { unsigned char(rgb.x * 255.0f), unsigned char(rgb.y * 255.0f), unsigned char(rgb.z * 255.0f), 255 };

			// Tmp scale is from -5 to 45. So to get the correct color add 5 to c to compensate for the -5
			float c = values[1][i] - 273.15f;
			rgb = getTMPColor((c + 5.0f) / 50.0f);
			colors[1][i] = { unsigned char(rgb.x * 255.0f), unsigned char(rgb.y * 255.0f), unsigned char(rgb.z * 255.0f), 255 };

			float b = (std::log(values[0][i] / 100.0f) + ((17.27 * c) / (237.3 + c))) / 17.27f;
			float dewC = (237.3f * b) / (1.0f - b);

			values[DEWPOINT_INDEX][i] = dewC;
			rgb = getTMPColor((dewC + 5.0f) / 50.0f);
			colors[DEWPOINT_INDEX][i] = { unsigned char(rgb.x * 255.0f), unsigned char(rgb.y * 255.0f), unsigned char(rgb.z * 255.0f), 255 };

			float lcl = 125 * (c - dewC);
			values[LCL_INDEX][i] = lcl;
			rgb = getTMPColor(lcl / 3000.0f);
			colors[LCL_INDEX][i] = { unsigned char(rgb.x * 255.0f), unsigned char(rgb.y * 255.0f), unsigned char(rgb.z * 255.0f), 255 };

			float cape = values[2][i];
			if (cape == 0.0f)
				colors[2][i] = { 0, 48, 99, 255 };
			else
			{
				rgb = getCAPEColor(values[2][i] / 2000.0f);
				colors[2][i] = { unsigned char(rgb.x * 255.0f), unsigned char(rgb.y * 255.0f), unsigned char(rgb.z * 255.0f), 255 };
			}

			values[3][i] = values[3][i] * 60.0f * 60.0f / 1000.0f;		// Convert from m/s to km/h
			rgb = getWindColor(values[3][i] / 100.0f);
			colors[3][i] = { unsigned char(rgb.x * 255.0f), unsigned char(rgb.y * 255.0f), unsigned char(rgb.z * 255.0f), 255 };
		}

		glCreateTextures(GL_TEXTURE_2D, NUM_PARAMS, textures);

		for (size_t i = 0; i < NUM_PARAMS; i++)
		{
			glTextureParameteri(textures[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(textures[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(textures[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(textures[i], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureStorage2D(textures[i], 1, GL_RGBA8, texWidth, texHeight);
			glTextureSubImage2D(textures[i], 0, 0, 0, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, colors[i].data());
		}
	}

	void Clear()
	{
		for (size_t i = 0; i < NUM_PARAMS; i++)
		{
			values[i].clear();
		}
	}

	void EnableLerp(bool enable)
	{
		if (enable)
		{
			for (size_t i = 0; i < NUM_PARAMS; i++)
			{
				glTextureParameteri(textures[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTextureParameteri(textures[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
		}
		else
		{
			for (size_t i = 0; i < NUM_PARAMS; i++)
			{
				glTextureParameteri(textures[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTextureParameteri(textures[i], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
		}
	}

	float GetValue(ParamIndices type, unsigned int widthIdx, unsigned int heightIdx) const
	{
		if (widthIdx >= 0 && widthIdx < width && heightIdx >= 0 && heightIdx < height)
			return values[type][heightIdx * width + widthIdx];
		
		return 0.0f;
	}

	size_t GetNumValues() const { return values[0].size(); }
	GLuint GetTexture(unsigned int idx) const { if (idx >= NUM_PARAMS || idx < 0) return 0; else return textures[idx]; }

private:
	std::vector<float> values[NUM_PARAMS];
	GLuint textures[NUM_PARAMS];
	unsigned int width;
	unsigned int height;
};

class TextGrib {
public:
	int Load(const std::string &fileName)
	{
		std::ifstream file(fileName);

		if (!file.is_open())
		{
			std::cout << "Failed load text grib: " << fileName << '\n';
			return -1;
		}

		std::string line;
		
		int idx = 0;
		int hourIdx = 0;
		bool first = true;
		bool firstRecord = true;
		bool firstAppearance = true;
		float firstLonValue = 0.0f;

		Record record;
		
		while (std::getline(file, line))
		{
			if (line.substr(0, 1) == "l")
			{
				if (!first)
				{
					records.push_back(record);
					record.Clear();
					firstRecord = false;
				}

				size_t idx1 = line.find_last_of(',');
				size_t idx2 = line.find_first_of(' ');
				std::string param = line.substr(idx1 + 1, idx2 - idx1 - 1);			// Add +1 to ommit the comma and -1 to ommit the space

				if (firstRecord)
				{
					std::string date = line.substr(line.find("d=") + 2, 14);
					year = std::stoi(date.substr(0, 4));
					month = std::stoi(date.substr(4, 2));
					day = std::stoi(date.substr(6, 2));
					startHour = std::stoi(date.substr(8, 2));

					dateStr = date.substr(6, 2);
					dateStr += '.' + date.substr(4, 2);
					dateStr += '.' + date.substr(0, 4) + ' ';
					dateStr += date.substr(8, 2) + 'h';
				}

				if (param == "RH")
				{
					first = true;
					idx = RH_INDEX;
				}
				else if (param == "TMP")
				{
					idx = TMP_INDEX;
				}
				else if (param == "CAPE")
				{
					idx = CAPE_INDEX;
				}
				else if (param == "GUST")
				{
					idx = GUST_INDEX;
					first = false;
				}

				hourIdx++;
			}
			else
			{
				float lon = std::stof(line.substr(0, line.find(',')));			// Get the longitude value

				if (firstAppearance)
				{
					firstLonValue = lon;
					firstAppearance = false;
				}
				if (lon == firstLonValue && firstRecord)				// Measure the number of occurrences of the first longitude value to calculate the height and width
					gridHeight++;

				record.AddValue(idx, std::stof(line.substr(21)));
			}
		}

		// Push back the last record
		records.push_back(record);

		gridHeight /= 4;
		size_t numValues = records[0].GetNumValues();
		gridWidth = numValues / gridHeight;

		for (size_t i = 0; i < records.size(); i++)
		{
			//	records[i].AddValue()

			records[i].Finish(gridWidth, gridHeight);
		}

		return 0;
	}

	void EnableLerp(bool enable)
	{
		for (size_t i = 0; i < records.size(); i++)
		{
			records[i].EnableLerp(enable);
		}
	}

	void Clear()
	{
		for (auto &r : records)
		{
			r.Clear();
		}
		records.clear();
		gridWidth = 0;
		gridHeight = 0;
	}

	const std::string &GetDateString() const { return dateStr; }
	int GetYear() const { return year; }
	int GetMonth() const { return month; }
	int GetDay() const { return day; }
	int GetStartHour() const { return startHour; }
	unsigned int GetGridWidth() const { return gridWidth; }
	unsigned int GetGridHeight() const { return gridHeight; }
	const std::vector<Record> &GetRecords() const { return records; }
	size_t GetNumRecords() const { return records.size(); }
	
private:
	int year = 0;
	int month = 0;
	int day = 0;
	int startHour = 0;
	std::string dateStr;

	unsigned int gridWidth = 0;
	unsigned int gridHeight = 0;
	std::vector<Record> records;
};

int main()
{
	unsigned int width = 800;
	unsigned int height = 640;

	Window window;
	window.Init(width, height);

	TextRenderer textRenderer;
	textRenderer.Init("Data/Fonts/arial_sdf.fnt", "Data/Fonts/arial_sdf.png");
	textRenderer.Resize(width, height);

	Profiler &profiler = window.GetProfiler();

	Quad q;
	q.Load();

	Shader quadShader;
	quadShader.Load("Data/Shaders/Weather/quad.vert", "Data/Shaders/Weather/quad.frag");

	TextGrib textGrib;
	textGrib.Load("latest.txt");

	GLuint map = utils::LoadTexture("Data/map.png", false);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	unsigned int texIdx = 0;
	bool isLinInterpEnabled = false;
	float opacity = 0.5f;
	int recordIdx = 0;
	std::string newDateStr;
	float value = 0.0f;

	while (!window.ShouldClose())
	{
		glm::vec2 pos = Input::GetMousePosition();
		float left = -1.0f * 0.45f;
		left = left * 0.5f + 0.5f;
		left *= float(width);

		float right = 1.0f * 0.45f;
		right = right * 0.5f + 0.5f;
		right *= float(width);

		float range = right - left;

		if (textGrib.GetNumRecords() > 0 && pos.x >= left && pos.x < right)
		{
			pos.x -= left;
			pos.x /= range;

			pos.y /= float(height);

			// flip the y
			pos.y = 1.0f - pos.y;

			int widthIdx = pos.x * textGrib.GetGridWidth();
			int heightIdx = pos.y * textGrib.GetGridHeight();

			//std::cout << widthIdx << "   " << heightIdx << '\n';

			value = textGrib.GetRecords()[recordIdx].GetValue((ParamIndices)texIdx, widthIdx, heightIdx);

			if (texIdx == TMP_INDEX)
				value -= 273.15f;
		}

		window.Update();
		if (window.WasResized())
		{
			width = window.GetWidth();
			height = window.GetHeight();
			glViewport(0, 0, width, height);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);

		if (textGrib.GetNumRecords() > 0)
		{
			quadShader.Use();
			quadShader.SetFloat("depth", 0.2f);
			quadShader.SetFloat("flipY", 0.0f);
			glBindTextureUnit(0, textGrib.GetRecords()[recordIdx].GetTexture(texIdx));
			q.Render();
		}

		quadShader.SetFloat("depth", 0.1f);
		quadShader.SetFloat("flipY", 1.0f);
		glBindTextureUnit(0, map);
		q.Render();


		profiler.BeginQuery("ImGui");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings", (bool*)0, 0);
		if (ImGui::Button("Download latest data"))
		{
			// Check if current grib is the latest
			auto now = std::chrono::system_clock::now();
			std::time_t time = std::chrono::system_clock::to_time_t(now);
			tm t;
			errno_t err = localtime_s(&t, &time);
			
			// Make sure the runs are available
			bool h0 = textGrib.GetStartHour() == 0 && t.tm_hour < 12;
			bool h6 = textGrib.GetStartHour() == 6 && t.tm_hour < 18;
			bool h12 = textGrib.GetStartHour() == 12 && t.tm_hour < 24;
			bool h18 = textGrib.GetStartHour() == 18 && t.tm_hour < 6;

			// If today is larger then the day in the grib then we need to download
			bool dayLarger = t.tm_mday > textGrib.GetDay();

			// If the month is larger then the month in the grib then we need to download
			bool monthLarger = t.tm_mon > textGrib.GetMonth();


			// Also make sure the we've loaded the data
			if (textGrib.GetNumRecords() > 0 && (h0 || h6 || h12 || h18) && !dayLarger && !monthLarger)
			{
				std::cout << "You already have the latest grib\n";
			}
			else
			{
				if (DownloadGRIB() != 0)
					return -1;
				if (ConvertGRIBToText() != 0)
					return -1;

				textGrib.Clear();
				if (textGrib.Load("latest.txt") != 0)
					return -1;
			}
		}
		ImGui::Text("Start Date:");
		ImGui::Text(textGrib.GetDateString().c_str());
		ImGui::Text("Forecast Date:");
		ImGui::Text(newDateStr.c_str());

		if (ImGui::SliderInt("Timestep", &recordIdx, 0, (int)textGrib.GetNumRecords() - 1))
		{
			int newHour = textGrib.GetStartHour() + recordIdx * 3;
			int newDay = textGrib.GetDay();
			int newMonth = textGrib.GetMonth();
			int newYear = textGrib.GetYear();

			if (newHour > 24)
			{
				newDay++;
				newHour -= 24;
			}

			newDateStr = std::to_string(newDay);
			newDateStr += '.' + std::to_string(newMonth);
			newDateStr += '.' + std::to_string(newYear) + ' ';
			newDateStr += std::to_string(newHour) + 'h';
		}

		if (isLinInterpEnabled)
		{
			if (ImGui::Button("Disable Lerp"))
			{
				textGrib.EnableLerp(false);
				isLinInterpEnabled = false;
			}
		}
		else
		{
			if (ImGui::Button("Enable Lerp"))
			{
				textGrib.EnableLerp(true);
				isLinInterpEnabled = true;
			}
		}
		
		const char* items[] = { "Relative Humidity", "Temperature", "CAPE", "Wind Gust", "Dew point", "LCL"/*, "TStorm Index"*/ };
		static const char* item_current = items[0];            // Here our selection is a single pointer stored outside the object.
		if (ImGui::BeginCombo("Parameter", item_current, 0)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (item_current == items[n]);
				if (ImGui::Selectable(items[n], is_selected))
				{
					item_current = items[n];
					texIdx = n;
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
			ImGui::EndCombo();
		}

		ImGui::Text(std::to_string(value).c_str());

		ImGui::End();

		/*ImGui::SetWindowSize(ImVec2(200, 400));
		ImGui::Begin("Skew-t", 0, ImGuiWindowFlags_NoScrollbar);

		ImVec2 rectpos = ImVec2(0.0f, 0.0f);
		ImVec2 size = ImGui::GetWindowSize();

		ImDrawList *drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(ImVec2(rectpos.x+10.0f, ImGui::GetFrameHeightWithSpacing() + rectpos.y), ImVec2(rectpos.x + size.x, rectpos.y + size.y), IM_COL32(255, 255, 255, 255));
		
		float i = 0.0f;
		for (int temp = 40; temp > -40; temp -= 5)
		{
			float x = size.x - 25.0f - 40.0f * i;
			if (x > 20.0f)
			{
				drawList->AddText(ImVec2(size.x - 25.0f - 40.0f * i, size.y - ImGui::GetTextLineHeight()), IM_COL32(0, 0, 0, 255), std::to_string(temp).c_str());
			}
			else


			}

			
			i += 1.0f;
		}
		

		ImGui::End();*/

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		profiler.EndQuery();

		profiler.BeginQuery("Text");
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);

		textRenderer.AddText("CPU time: " + std::to_string(window.GetDeltaTime() * 1000.0f) + " ms", glm::vec2(30.0f, 460.0f), glm::vec2(0.3f, 0.3f));
		textRenderer.AddText(profiler.GetResults(), glm::vec2(30.0f, 420.0f), glm::vec2(0.3f, 0.3f));
		textRenderer.Render();
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

		profiler.EndQuery();
		profiler.EndFrame();

		window.SwapBuffers();
	}

	q.Dispose();

	window.Dispose();

	return 0;
}
