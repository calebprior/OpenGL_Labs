#pragma once

#pragma region INCLUDES

#ifndef MAP
#define MAP

#if ! defined(STB_IMAGE_IMPLEMENTATION)
#include "stb_image.h"
#endif

#include <iostream>

#pragma endregion INCLUDES

namespace Types {
	enum Types {
		Default,
		Ground,
		Bush,
		Well,
		Light,
		TombStone1,
		TombStone2,
		Pedestal,
		Flag,
		End
	};

}
class Map {
public:
	std::vector<std::vector<int>> map;
	int width, height, n;

	std::vector<vec3> lightModelPositions;
	std::vector<vec2> lightPositions;

	vec3 playerStartPos;

	bool atEnd = false;

	enum STB_COLORS {
		WHITE = 255,
		RED = 76,
		GREEN = 149,
		BLUE = 28,
		PINK = 105,
		YELLOW = 226,
		CYAN = 178,
		GREY = 100, // 100, 100, 100
		GREY2 = 150 // 150, 150, 150
	};
	char* path;

	void setupMap() {
		unsigned char* image = stbi_load(this->path, &width, &height, &n, 1);

		map.resize(width, std::vector<int>(height, 0));

		std::cout << width << " " << height << std::endl;

		float randNum;

		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				int index = i * width + j;
				int val = image[index];

				switch (val)
				{
				case WHITE:
					map[i][j] = Types::Ground;
					break;
				case RED:
					map[i][j] = Types::Ground;
					playerStartPos = vec3(j + 0.5, 1.5, -(((height - 1) - i) + 0.5));
					break;
				case GREEN:
					map[i][j] = Types::Bush;
					break;
				case BLUE:
					map[i][j] = Types::Well;
					break;
				case PINK:
					map[i][j] = Types::End;
					break;
				case YELLOW:
					map[i][j] = Types::Light;
					lightModelPositions.push_back(vec3(j + 0.5, 2.0, -(((height - 1) - i) + 0.5)));
					lightPositions.push_back(vec2(i, j));
					break;
				case CYAN:
					randNum = (float)rand() / (float)RAND_MAX;
					randNum *= 2;

					if (randNum < 1) {
						map[i][j] = Types::TombStone1;
					}
					else {
						map[i][j] = Types::TombStone2;
					}
					break;
				case GREY:
					map[i][j] = Types::Pedestal;
					break;
				case GREY2:
					map[i][j] = Types::Flag;
					break;
				default:
					if (val != 0) {
						std::cout << val << std::endl;
					}
					map[i][j] = Types::Default;
					break;
				}
			}
		}
	}

	Map(char* path = "") {
		if (path != "") {
			this->path = path;
			setupMap();

			SetupLightMap();
		}
	}

	bool canMove(vec2 newPos) {
		int x = (int)(newPos.v[0] + 0.1);
		int y = (int)(height - 1 + (int)(newPos.v[1] - 0.2));

		int x2 = (int)(newPos.v[0] - 0.2);
		int y2 = (int)(height - 1 + (int)(newPos.v[1] + 0.2));

		bool result1 = false;
		bool result2 = false;


		if (y >= 0 && y < height && x >= 0 && x < width) {
			result1 = (map[y][x] == Types::Bush) || (map[y][x] == Types::Well || 
				(map[y2][x2] == Types::Pedestal) || (map[y2][x2] == Types::TombStone1) || 
				(map[y2][x2] == Types::TombStone2));

			atEnd = map[y][x] == Types::End;
		}

		if (y2 >= 0 && y2 < height && x2 >= 0 && x2 < width) {
			result2 = (map[y2][x2] == Types::Bush) || (map[y2][x2] == Types::Well) || 
				(map[y2][x2] == Types::Pedestal || (map[y2][x2] == Types::TombStone1) || 
					(map[y2][x2] == Types::TombStone2));

			atEnd = map[y][x] == Types::End;
		}

		if (result1 || result2) {
			return false;
		}
		else {
			return true;
		}
	}

	void printMap(){
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				std::cout << map[i][j] << " ";
			}

			std::cout << std::endl;
		}
	}

	void printLightMap() {
		std::cout << "Max: " << maxNumLights << std::endl;
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				std::cout << "|";
				for (int k = 0; k < lightMap[i][j].size(); k++) {
					std::cout << lightMap[i][j][k] << " ";
				}
			}

			std::cout << std::endl;
		}
	}

	std::vector<std::vector<std::vector<int>>> lightMap;
	int offset = 2;
	int maxNumLights = 0;

	void SetupLightMap() {
		lightMap.resize(width, std::vector<std::vector<int>>(height));
		
		int lightCount = 0;

		for (int i = 0; i < lightPositions.size(); i++) {
			int starti = lightPositions[i].v[0] - offset;
			int startj = lightPositions[i].v[1] - offset;

			if (starti < 0) {
				starti = 0;
			}

			if (startj < 0) {
				startj = 0;
			}

			int endi = starti + 2 * offset;
			int endj = startj + 2 * offset;

			if (endi >= map.size()) {
				endi = map.size() - 1;
				starti = endi - 2 * offset;
			}

			if (endj >= map[endi].size()) {
				endj = map[endi].size() - 1;
				startj = endj - 2 * offset;
			}

			int temp = startj;
			while (starti <= endi) {
				startj = temp;
				while (startj <= endj) {
					lightMap[starti][startj].push_back(lightCount);

					if (lightMap[starti][startj].size() > maxNumLights) {
						maxNumLights = lightMap[starti][startj].size();
					}

					startj++;
				}
				starti++;
			}

			lightCount++;
		}
	}

private:

};

#endif