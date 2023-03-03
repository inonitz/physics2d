#include "test.hpp"
#include "../hashtable.hpp"
#include "../vec.hpp"
#include <chrono>
#include <memory>
#include <ratio>
#include <vector>
#include <filesystem>





template<u8 type> struct GridCell {
	f32 p;
	f32 v;
	math::vec2f ux;
	math::vec2f uy;
	math::vec2f uz;
};


#define __MEASURE(timer_struct, func) \
	timer_struct.tick(); \
	func; \
	timer_struct.tock(); \
\


void test_hashtable(std::vector<BenchmarkResult>& to_append, Timer<>& timer, NumberGenerator<>& rng, FILE* to_save)
{

	Hashtable<size_t, GridCell<0>> table;
	std::array<BenchmarkResult::chrono_ns, 4> opTime;
	std::array<bool, 4> 					  opStatus = {1, 1, 1, 1};
	std::array<u64, 4> 						  opKeys = {0, 0, 0, 0};
	f32 				  					  avgOpTime{0.0f}; 
	BenchmarkResult res;

	size_t toGenerate = 1000, axisX = 360, axisY = 360;
	std::vector<GridCell<0>> randCells{toGenerate}; 
	GridCell<0>& selectedCell = randCells[0];
	u64 	     key 		  = 0;

	
	to_append.resize(10 + axisX * axisY / 4);
	table.overrideFunctors(
		[](size_t key) -> u64 { static const u64 seed = readTimestampCounter(); return MurmurHash64A(&key, 8, seed); },
		[](size_t k0, size_t k1) -> bool { return k0 == k1; }
	);
	__MEASURE(timer, table.create(134798, 67399)); /* 67399 is prime. */
	

	res.measured_ns = __scast(f32, timer.duration<BenchmarkResult::chrono_ns>().count());
	std::snprintf(res.name.data(), 16, "table::create()");
	to_append.push_back(res);


	alignsz(16) std::array<f32, 4> randVals;
	for(size_t i = 0; i < toGenerate; ++i) {
		randVals = { rng.randf(), rng.randf(), rng.randf(), rng.randf() };
		 randCells[i] = {
			rng.randf(),
			randVals[0],
			math::vec2f{ randVals[1] },
			math::vec2f{ randVals[2] },
			math::vec2f{ randVals[3] }
		};
	}

	/* Test Insertion. */
	printf("Insert() Progress: ");
	std::array<char, 200> resultString = {0};
	size_t progress = 0;
	size_t inserted = 0;
	size_t found    = 0;
	size_t i = 0, j = 0;
	for(i = 0; i < axisX; ++i) {
		for(j = 0; j < axisY; j += 4) {
			for(size_t k = 0; k < 4; ++k) 
			{
				key = (j+k) + i * axisX;
				selectedCell = randCells[rng.randu() % randCells.size()];
				__MEASURE(timer, opStatus[k] = table.insert(key, selectedCell));
				opKeys[k] = key;
				opTime[k] = timer.duration<>();
				avgOpTime += timer.duration_cast<f32>(opTime[k]).count();
			}
			avgOpTime *= 0.25f;
			progress += 4;
			
			std::snprintf(resultString.data(), sizeof(resultString), "[Insert] [Key, Status, Time] [%6llu, %6llu, %6llu, %6llu] [ %u , %u , %u , %u ] [ %5.3f, %5.3f, %5.3f, %5.3f ]", 
				opKeys[0],	  opKeys[1], 	opKeys[2], 	 opKeys[3],
				opStatus[0], opStatus[1], opStatus[2], opStatus[3],
				timer.duration_cast<f32>(opTime[0]).count(),
				timer.duration_cast<f32>(opTime[1]).count(),
				timer.duration_cast<f32>(opTime[2]).count(),
				timer.duration_cast<f32>(opTime[3]).count()
			);
			inserted += (size_t)opStatus[0] + (size_t)opStatus[1] + (size_t)opStatus[2] + (size_t)opStatus[3];
			res = {
				resultString,
				avgOpTime,
				BENCHMARK_STATUS_FINISHED
			};
			to_append.push_back(res);
			avgOpTime = 0;
			printf("%3.2f%%\b\b\b\b\b\b", __scast(f32, progress) / toGenerate);
		}
	}


	std::snprintf(resultString.data(), sizeof(resultString), "[NOTICE] [Inserted %llu/%llu]", inserted, axisX * axisY);
	res = {
		resultString,
		0.0f,
		BENCHMARK_STATUS_FINISHED | BENCHMARK_STATUS_SUCCESS
	};
	to_append.push_back(res);
	printf("\n");


	printf("Lookup() Progress: ");
	key = 0;
	opStatus = { true, true, true, true };
	opKeys   = { 0, 0, 0, 0 };
	for(i = 0; i < axisX; ++i) {
		for(j = 0; j < axisY; j += 4) {
			for(size_t k = 0; k < 4; ++k) 
			{
				key = (j+k) + i * axisX;
				__MEASURE(timer, opStatus[k] = boolean(table.lookup(key)));
				opKeys[k] = key;
				opTime[k] = timer.duration<>();
				avgOpTime += timer.duration_cast<f32>(opTime[k]).count();
			}
			avgOpTime /= 4;
			progress += 4;

			std::snprintf(resultString.data(), sizeof(resultString), "[LookUp] [Key, Status, Time] [%6llu, %6llu, %6llu, %6llu] [ %u , %u , %u , %u ] [ %5.3f, %5.3f, %5.3f, %5.3f ]", 
				opKeys[0],	   opKeys[1], 	opKeys[2], 	 opKeys[3],
				opStatus[0], opStatus[1], opStatus[2], opStatus[3],
				timer.duration_cast<f32>(opTime[0]).count(),
				timer.duration_cast<f32>(opTime[1]).count(),
				timer.duration_cast<f32>(opTime[2]).count(),
				timer.duration_cast<f32>(opTime[3]).count()
			);
			found += (size_t)opStatus[0] + (size_t)opStatus[1] + (size_t)opStatus[2] + (size_t)opStatus[3];
			res = {
				resultString,
				avgOpTime,
				BENCHMARK_STATUS_FINISHED
			};
			to_append.push_back(res);
			avgOpTime = 0.0f;
			printf("%3.2f%%\b\b\b\b\b\b", __scast(f32, progress) / toGenerate);
		}
	}
	printf("\n");
	std::snprintf(resultString.data(), sizeof(resultString), "[NOTICE] [LookUp Found %llu/%llu]", found, axisX * axisY);
	res = {
		resultString,
		0.0f,
		BENCHMARK_STATUS_FINISHED | BENCHMARK_STATUS_SUCCESS
	};
	to_append.push_back(res);


	if(to_save != nullptr)
		table.to_file(to_save);
	
	__MEASURE(timer, table.destroy());
	res.measured_ns = __scast(f32, timer.duration<BenchmarkResult::chrono_ns>().count());
	std::snprintf(res.name.data(), 16, "table::destroy()");
	to_append.push_back(res);
	return;
}


// void test_table_insert(Hashtable<size_t, GridCell<0>>& table, std::vector<BenchmarkResult>& results, Timer<>& timer, NumberGenerator<>& rng, FILE* to_save);
// void test_table_lookup(Hashtable<size_t, GridCell<0>>& table, std::vector<BenchmarkResult>& results, Timer<>& timer, NumberGenerator<>& rng, FILE* to_save);
// void test_table_delete(Hashtable<size_t, GridCell<0>>& table, std::vector<BenchmarkResult>& results, Timer<>& timer, NumberGenerator<>& rng, FILE* to_save);



int testing()
{
	std::vector<Benchmark> tests;
	Timer<> 		  	   timer{};
	NumberGenerator<> 	   rng{};
	const char* 		   path = "C:/Program Files/Programming Utillities/CProjects/mglw-strip/src/benchmarker.txt";
	FILE*                  ResultTxt = fopen(path, "w+");;
	tests.resize(10);



	ifcrashfmt(ResultTxt == nullptr, "Couldn't get handle to file at Path[%s]\n", path);
	tests[0] = {
		"Hashtable Tests",
		std::make_unique<std::vector<BenchmarkResult>>(),
	};
	markstr("TESTING HASHTABLE ... ");
	test_hashtable(*tests[0].data, timer, rng, ResultTxt);
	markstr(" FINISHED ");


	u32 id = 0, bmid = 0;
	for(auto& bm : tests) {
		if(bm.data == nullptr)
			continue;
		
		fprintf(ResultTxt, "[%u] Benchmark %s:\n", id, bm.title.data());
		for(auto& result : *bm.data) {
			fprintf(ResultTxt, "    %s [%u/%llu] Measured %5.3fns\n", result.name.data(), bmid, bm.data->size(), result.measured_ns);
			++bmid;
		}
		++id;
		bmid = 0;
	}


	fclose(ResultTxt);
	markstr("Program Exit Success\n");
	return 0;
}
