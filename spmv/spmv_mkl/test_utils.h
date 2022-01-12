#include <stdio.h>
#include <papi.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <assert.h>

#include <vector>
#include <set>

#define CHECK_PAPI_VALID(stat, val) do {\
    int retval = stat;\
    if(retval != val) { fprintf(stderr, "Error at %s:%d: %s, return val=%d\n", __FILE__, __LINE__, #stat, retval); exit(retval); }\
} while(0)

bool papi_initialized=false;

int __PAPI_AVAIL_NUM = 0;
int __PAPI_SLICE_NUM = 0;
double* __PAPI_SLICE = NULL;
std::vector<std::string> event_names;
int* __PAPI_EVENT_SETS= NULL;

int PAPI_PMU_MAX_NUM;
#define MAX_PMU_NUM 10

// #define PAPI_PMU_MAX_NUM 4
#define SAFE_FREE(ptr) do { \
    if(ptr) { free(ptr); ptr=NULL; } \
} while(0)

static bool is_derived( PAPI_event_info_t * info ) {
    if ( strlen( info->derived ) == 0 )
        return false;
    else if ( strcmp( info->derived, "NOT_DERIVED" ) == 0 )
        return false;
    else if ( strcmp( info->derived, "DERIVED_CMPD" ) == 0 )
        return false;
    else
        return true;
}

void papi_init() {
    CHECK_PAPI_VALID(PAPI_library_init( PAPI_VER_CURRENT ), PAPI_VER_CURRENT);
    // split all available papi events into several subsets
    std::vector<int> event_codes;
    int event = 0 | PAPI_PRESET_MASK;
    int num = PAPI_get_opt( PAPI_MAX_HWCTRS, NULL );
    CHECK_PAPI_VALID(PAPI_enum_event( &event, PAPI_ENUM_FIRST ), PAPI_OK);
    PAPI_event_info_t info;
    do {
        int retval = PAPI_get_event_info(event, &info);
        if (retval == PAPI_OK && !is_derived(&info)) {
            event_names.push_back(info.symbol);
            event_codes.push_back(info.event_code);
            // printf("Collecting event: %x: %s\n", event, info.symbol);
        }
    } while (PAPI_enum_event(&event, PAPI_PRESET_ENUM_AVAIL) == PAPI_OK);
    __PAPI_AVAIL_NUM = event_codes.size();
    assert(__PAPI_AVAIL_NUM>0);

    int i;
    int EventSet = PAPI_NULL;
    CHECK_PAPI_VALID( PAPI_create_eventset(&EventSet), PAPI_OK );
    for(i=0;i<num && i<__PAPI_AVAIL_NUM;++i) {
        int retval = PAPI_add_event(EventSet, event_codes[i]);
        if(retval!=PAPI_OK) {
            break;
        }
    }
    PAPI_PMU_MAX_NUM = i;

    if(PAPI_PMU_MAX_NUM==0) {
            printf("papi_init failed: PAPI_PMU_MAX_NUM == 0!\n");
            printf("INFO: ignore PAPI collection!\n");
            return;
    }

    __PAPI_SLICE_NUM = (__PAPI_AVAIL_NUM+PAPI_PMU_MAX_NUM-1) / PAPI_PMU_MAX_NUM;
    __PAPI_SLICE = (double*)malloc(sizeof(double)*__PAPI_SLICE_NUM*PAPI_PMU_MAX_NUM);
    __PAPI_EVENT_SETS = (int*)malloc(sizeof(int)*__PAPI_SLICE_NUM);
    assert(__PAPI_SLICE != NULL);
    assert(__PAPI_EVENT_SETS != NULL);

    __PAPI_SLICE[0] = 0.0;
    __PAPI_EVENT_SETS[0] = EventSet;

    printf("Available number of hardware counters: %d\n", PAPI_PMU_MAX_NUM);
    printf("PAPI Avail Event Num = %d\n", __PAPI_AVAIL_NUM);
    printf("Number of tests need for collecting all PAPI counter values: %d\n", __PAPI_SLICE_NUM);

    for(i=1; i<__PAPI_SLICE_NUM; ++i) {
        __PAPI_EVENT_SETS[i] = PAPI_NULL;
        CHECK_PAPI_VALID( PAPI_create_eventset(__PAPI_EVENT_SETS+i), PAPI_OK );
        for(int k=0; k<PAPI_PMU_MAX_NUM; ++k) {
            uint32_t idx = i*PAPI_PMU_MAX_NUM+k;
            if(idx < event_codes.size()) {
                CHECK_PAPI_VALID( PAPI_add_event(__PAPI_EVENT_SETS[i], event_codes[idx]), PAPI_OK );
            }
            __PAPI_SLICE[idx] = 0.0;
        }
    }
    papi_initialized = true;
}

void papi_fini() {
    if(!papi_initialized) return;
    for(int i=0; i<__PAPI_SLICE_NUM; ++i) {
        PAPI_destroy_eventset(__PAPI_EVENT_SETS+i);
    }
    __PAPI_SLICE_NUM = 0;
    __PAPI_AVAIL_NUM = 0;
    SAFE_FREE(__PAPI_SLICE);
    SAFE_FREE(__PAPI_EVENT_SETS);
}

void test_begin(int i) {
    CHECK_PAPI_VALID(PAPI_start( __PAPI_EVENT_SETS[i] ), PAPI_OK);
}

void test_end(int i, int scale) {
    long long int value[MAX_PMU_NUM] = {0};
    CHECK_PAPI_VALID(PAPI_stop( __PAPI_EVENT_SETS[i], value ), PAPI_OK);
    double* cur_papi_slice = __PAPI_SLICE + i*PAPI_PMU_MAX_NUM;
    for(int k=0; k<PAPI_PMU_MAX_NUM; ++k) {
        cur_papi_slice[k] = (double)value[k] / (double)scale;
    }
}

// output metrics to <name>.dat
void test_output(const char* name) {
    std::string fn(name);
    fn += ".dat";
    FILE* fp = fopen(fn.c_str(), "w");
    if(fp==NULL) {
        fprintf(stderr, "Failed to open file: %s", fn.c_str());
    }

    for(int i=0;i<__PAPI_AVAIL_NUM;++i) {
        fprintf(fp, "%s,%lf\n", event_names[i].c_str(), __PAPI_SLICE[i]);
    }

    fclose(fp);
}

std::string remove_extension(const std::string& filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot); 
}

std::vector<std::string> splitpath(
  const std::string& str
  , const std::set<char> delimiters=std::set<char>({'/'})) {
    std::vector<std::string> result;
    char const* pch = str.c_str();
    char const* start = pch;
    for(; *pch; ++pch) {
        if (delimiters.find(*pch) != delimiters.end()) {
            if (start != pch) {
                std::string str(start, pch);
                result.push_back(str);
            }
            else {
                result.push_back("");
            }
            start = pch + 1;
        }
    }
    result.push_back(start);
    return result;
}



#define PAPI_TEST_EVAL(WARM_TIMES, EVAL_TIMES, flops, name, stat) do {\
    for(int i=0;i<WARM_TIMES;++i) { stat; }\
    if(!papi_initialized) {\
      for(int i=0;i<EVAL_TIMES;++i) { stat; }\
    } else {\
      for(int k=0;k<__PAPI_SLICE_NUM;++k) {\
        /*printf("Running %d-th (total %d) event set...\n", k, __PAPI_SLICE_NUM);*/ \
        test_begin(k);\
        for(int i=0;i<EVAL_TIMES;++i) { stat; }\
        test_end(k, EVAL_TIMES);\
      }\
      test_output(name);\
    }\
} while(0)

