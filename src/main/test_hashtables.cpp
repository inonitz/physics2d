#include "test_hashtables.hpp"
#include "../hash.hpp"
#include "../random.hpp"
#include "../measure.hpp"
#include "../linked_hash.hpp"
#include "../flat_hash.hpp"
#include "../vec.hpp"
#include <filesystem>




#define __MEASURE(timer_structure, func) \
	timer_structure.tick(); \
	func; \
	timer_structure.tock(); \
\




template<u8 type> struct GridCell {
	f32 p;
	f32 v;
	math::vec2f ux;
	math::vec2f uy;
	math::vec2f uz;
};


struct Hash {
	size_t seed;

	Hash() : seed{random64u()} {}
	size_t operator()(u64 key) { return MurmurHash64A(&key, 8, seed); }
	void refresh() { 
		markfmt("hash::refresh() switched to new seed=0x%llX\n", seed);
		seed = random64u(); return;
	}
};




static Timer<> 		  	 		   	     timer{};
static flat_hash<u64, GridCell<0>, Hash> table;

/* For Insert */
static size_t randCellCount = 256;
static std::vector<GridCell<0>> randCells(randCellCount);

/* stdout not big enough. */
static FILE* bench_data = fopen("benchmarker.txt", "w+");

/* lambda function for printing */
static auto printTableElement = [](size_t key, GridCell<0>) -> std::string {
	std::string out{22, ' '};
	std::snprintf(out.data(), out.max_size(), "{ k=%3llu }", key);
	return out;
};




int testing_hashtables()
{
	table.create(192, 0);


	for(auto& e : randCells) 
	{
		static std::array<f32, 5> tmp = {
			random32f(),
			random32f(),
			random32f(),
			random32f(),
			random32f(),
		};
		e = {  tmp[0], tmp[1], 
			math::vec2f(tmp[2]),
			math::vec2f(tmp[3]),
			math::vec2f(tmp[4])
		};
	}


	/* Test Insertion. */
	u8 failed   = 0;
	u32 newSize = table.buckets();
	for(size_t i = 0; i < randCellCount; ++i)
	{
		__MEASURE(timer, failed = table.insert(i, randCells[i]));
		fprintf(bench_data, "Insert [%3llu=%s] [key=%3llu, value_ptr=%p] Took %lluns\n", i, table.statusToString(0, failed), i, (void*)&randCells[i], timer.duration().count());
		i -= failed > 0; /* we repeat the test at i if it didn't succeed. */

		while(failed > 0) { /* meaning NOT successful. */ 
			newSize = (newSize * 3) / 2;
			markfmt("newSize computed=%u\n", newSize);

			__MEASURE(timer, { failed = table.rehash(newSize); }); 
			fprintf(bench_data, "Insert [%3llu=%s re-hash Took %lluns]\n", i, table.statusToString(3, failed), timer.duration().count());

			failed = (failed == 0);
		}
	}
	table.to_file(printTableElement, bench_data);


	/* Test Lookups. */
	GridCell<0>* val;
	size_t k = 0;
	for(size_t i = 0; i < randCellCount; ++i)
	{
		k = random32u() % randCellCount + (random32u() & 0xFF);
		__MEASURE(timer, val = table.lookup(k));
		fprintf(bench_data, "Lookup   [%3llu=%s] [key=%3llu, value_ptr=%p] Took %lluns\n", i, table.statusToString(1, val == nullptr), k, (void*)&randCells[k], timer.duration().count());
	}


	/* Make sure inserts were successful. */
	k = 0;
	for(size_t i = 0; i < randCellCount; ++i)
	{
		k += table.lookup(i) != nullptr;
	}
	debug_messagefmt("found %llu/%llu inserted elements.\n", k, randCellCount);


	/* Test Deletion. */
	for(size_t i = 0; i < randCellCount; ++i)
	{
		k = random32u() % randCellCount + (random32u() & 0xFF);
		__MEASURE(timer, failed = table.del(k));
		fprintf(bench_data, "Deletion [%3llu=%s] [key=%3llu, value_ptr=%p] Took %lluns\n", i, table.statusToString(2, failed), k, (void*)&randCells[k], timer.duration().count());
	}
	table.to_file(printTableElement, bench_data);




	fclose(bench_data);
	return 0;
}
