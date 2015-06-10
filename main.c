#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false )
#else
#define DEBUG_PRINT(...) do{ } while ( false )
#endif


/**
 * Model defnitions
 */

typedef struct {
  id_t id;
  id_t model_id;
  char *name;
  bool is_zero_state;
} state_t;

typedef struct {
  id_t id;
  char *name;
  state_t *zero_state;
} model_t;

typedef struct {
  id_t cycle_id;
  id_t person_id;
  id_t state_id;
  id_t model_id;
} master_record_t;

typedef struct {
  id_t id;
  char *name;
} cycle_t;

typedef struct {
  id_t id;
} person_t;

typedef struct {
  id_t id;
  id_t cur_state_id;
  id_t from_state_id;
  id_t to_state_id;
  float adjustment;
  id_t effected_model_id;
} interaction_t;

typedef struct {
  id_t id;
  id_t from_id;
  id_t to_id;
  float prob;
} trans_prob_t;

/* EOF model definitions */


/**
 * Constant initializations
 * all model `id` fields need match the instance's position in model array
 */

model_t models[] = {
  { 0, "HIV" },
  { 1, "TB" }
};

state_t states[] = {
  { 0, 0, "Uninit", true },
  { 1, 0, "HIV-",   false },
  { 2, 0, "HIV+",   false },
  { 3, 1, "Uninit", true },
  { 4, 1, "TB-",    false },
  { 5, 1, "TB+",    false }
};

trans_prob_t trans_probs[] = {
  { 0,  0, 0, 0 },
  { 1,  0, 1, 0.99 },
  { 2,  0, 2, 0.01 },
  { 3,  1, 0, 0 },
  { 4,  1, 1, 0.95 },
  { 5,  1, 2, 0.05 },
  { 6,  2, 0, 0 },
  { 7,  2, 1, 0 },
  { 8,  2, 2, 1 },
  { 9,  3, 3, 0 },
  { 10, 3, 4, 0.8 },
  { 11, 3, 5, 0.2 },
  { 12, 4, 3, 0 },
  { 13, 4, 4, 0.9 },
  { 14, 4, 5, 0.1 },
  { 15, 5, 3, 0 },
  { 16, 5, 4, 0 },
  { 17, 5, 5, 1 }
};

interaction_t interactions[] = {
  { 0, 2, 4, 5, 2, 1 }
};

cycle_t cycles[] = {
  { 0,  "Pre-initialization" },
  { 1,  "2015" },
  { 2,  "2016" },
  { 3,  "2017" },
  { 4,  "2018" },
  { 5,  "2018" },
  { 6,  "2018" },
  { 7,  "2018" },
  { 8,  "2018" },
  { 9,  "2018" },
  { 10, "2018" },
  { 11, "2018" },
  { 12, "2018" },
  { 13, "2018" },
  { 14, "2018" },
  { 15, "2018" },
  { 16, "2018" },
  { 17, "2018" },
  { 18, "2018" },
  { 19, "2018" }
};

/* EOF constants */


// maybe should CAPITALIZE these consts
const size_t PERSON_COUNT = 100000;
const size_t STATE_COUNT = sizeof(states) / sizeof(state_t);
const size_t MODEL_COUNT = sizeof(models) / sizeof(model_t);
const size_t CYCLE_COUNT = sizeof(cycles) / sizeof(cycle_t);
const size_t TRANS_PROB_COUNT  = sizeof(trans_probs) / sizeof(trans_prob_t);
const size_t INTERACTION_COUNT = sizeof(interactions) / sizeof(interaction_t);

person_t *people;
master_record_t *masters;
uint master_count = 0;


/**
 * Indexes
 */

typedef struct {
    size_t x_size;
    size_t y_size;
    size_t z_size;
    id_t *data;
} index_3d_t;

uint index_3d_offset(index_3d_t *index, uint z, uint y, uint x)
{
    return ( z * index->x_size * index->y_size ) +
           ( y * index->x_size ) +
           x;
}

id_t index_3d_get(index_3d_t *index, uint z, uint y, uint x)
{
    uint offset = index_3d_offset(index, x, y, z);
    return index->data[offset];
}

id_t *index_3d_get_2d(index_3d_t *index, uint z, uint y)
{
    uint offset = ( z * index->x_size * index->y_size ) +
                  ( y * index->x_size );
    return &index->data[offset];
}

void index_3d_put(index_3d_t *index, uint z, uint y, uint x, id_t val)
{
    uint offset = index_3d_offset(index, x, y, z);
    index->data[offset] = val;
}

void index_3d_init(index_3d_t *index, size_t z, size_t y, size_t x)
{
    index->x_size = x;
    index->y_size = y;
    index->z_size = z;
    index->data = malloc(x * y * z * sizeof(*index->data));
}

index_3d_t state_id_by_cycle_person_model;
trans_prob_t **trans_probs_by_state[STATE_COUNT];
size_t state_get_dest_count(state_t *);

void init_trans_probs_by_state(void)
{
    /* for every state */
    for (int sid = 0; sid < STATE_COUNT; ++sid)
    {
        id_t model_id = states[sid].model_id;
        size_t dest_count = state_get_dest_count(&states[sid]);
        /* will store refs to all transitions from this state */
        trans_prob_t **value = malloc(dest_count * sizeof(trans_prob_t*));

        uint i = 0;
        /* for every transition probability */
        for (int tb_id = 0; tb_id < TRANS_PROB_COUNT; ++tb_id)
        {
            trans_prob_t *tb = &trans_probs[tb_id];
            if (tb->from_id == sid)
            {
                value[i++] = tb;
            }
        }
        trans_probs_by_state[sid] = value;
    }
}

void init_indexes(void)
{
    index_3d_init(&state_id_by_cycle_person_model, CYCLE_COUNT, PERSON_COUNT, MODEL_COUNT);
    init_trans_probs_by_state();
}

/* EOF indexes */


/**
 * Master records helpers
 */

void init_masters(void)
{
    size_t count = CYCLE_COUNT * MODEL_COUNT * STATE_COUNT * PERSON_COUNT;
    masters = malloc(count * sizeof(master_record_t));
}

void masters_push(id_t cycle_id, id_t state_id, id_t model_id, id_t person_id)
{
    DEBUG_PRINT("run_cycle(): cycle_id=%d person_id=%d model_id=%d state_id=%d\n", cycle_id, person_id, model_id, state_id);
    masters[master_count].cycle_id = cycle_id;
    masters[master_count].state_id = state_id;
    masters[master_count].model_id = model_id;
    masters[master_count].person_id = person_id;
    master_count += 1;
}


/**
 * Person helpers
 */

void person_add_state(id_t person_id, id_t cycle_id, state_t *state)
{
    index_3d_put(
        &state_id_by_cycle_person_model,
        cycle_id, person_id, state->model_id,
        state->id
    );
    masters_push(cycle_id, state->id, state->model_id, person_id);
}

state_t *person_get_state(id_t person_id, id_t cycle_id, id_t model_id)
{
    id_t id = index_3d_get(
        &state_id_by_cycle_person_model,
        cycle_id, person_id, model_id
    );
    return &states[id];
}

id_t *person_get_states(id_t person_id, id_t cycle_id)
{
    return index_3d_get_2d(
        &state_id_by_cycle_person_model,
        cycle_id, person_id
    );
}


/**
 * Model helpers
 */

 void init_models(void)
 {
     /* map model to model's zero state */
     /* maybe this should be considered an index and go to init_indexes() */
     for (int i = 0; i < STATE_COUNT; ++i)
     {
         if (states[i].is_zero_state)
         {
             models[states[i].model_id].zero_state = &states[i];
         }
     }
 }

size_t model_state_counts[MODEL_COUNT] = {0};

size_t model_get_state_count(id_t model_id)
{
    if (model_state_counts[model_id] == 0)
    {
        for (int sid = 0; sid < STATE_COUNT; ++sid)
        {
            if (states[sid].model_id == model_id)
            {
                model_state_counts[model_id] += 1;
            }
        }
    }
    return model_state_counts[model_id];
}


/**
 * State helpers
 */

size_t state_get_dest_count(state_t *state)
{
    return model_get_state_count(state->model_id);
}


/* also inits people */
void init_world(void)
{
    people = malloc(PERSON_COUNT * sizeof(person_t));

    /* for every person */
    for (int pid = 0; pid < PERSON_COUNT; ++pid)
    {
        people[pid].id = pid;
        /* for every model */
        for (int mid = 0; mid < MODEL_COUNT; ++mid)
        {
            /* get model's zero state */
            state_t *zero_state = models[mid].zero_state;
            /* assign zero state to person */
            person_add_state(pid, 0, zero_state);

        }
    }
}

// http://stackoverflow.com/questions/2509679/how-to-generate-a-random-number-from-within-a-range
uint rand_interval(uint min, uint max)
{
    int r;
    const uint range = 1 + max - min;
    const uint buckets = RAND_MAX / range;
    const uint limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

void suffle(uint *items, size_t len)
{
    // Knuth shuffle
    // http://stackoverflow.com/questions/14985737/how-to-access-a-c-array-in-random-order-ensuring-that-all-elements-are-visited-a
    for (int i = len-1; i > 1; --i)
    {
        uint j = rand_interval(0, i);
        uint itemi = items[i];
        items[i] = items[j];
        items[j] = itemi;
    }
}

/**
 * malloc the data structure that will be used to store probabilities
 * and destination state_ids during model runs. The `prob` field will
 * be adjusted if need by by interactions.
 * The size of this variable will be the count of the number of states
 * of the model with the most states.
 */
trans_prob_t *malloc_adjusted_probs(void)
{
    size_t max = 0;
    for (int mid = 0; mid < MODEL_COUNT; ++mid)
    {
        size_t count = model_get_state_count(mid);
        if (count > max)
        {
            max = count;
        }
    }
    return malloc(max * sizeof(trans_prob_t));
}

/**
 * @param adj_probs      array of transition probabilities that will be altered
 *                       during modelling
 * @param starting_probs array of transition probabilities that will be copied
 *                       to `adj_probs` as starting values.
 */
void init_adjusted_probs(trans_prob_t *adj_probs, trans_prob_t **starting_probs, size_t starting_probs_count)
{
    for (int i = 0; i < starting_probs_count; ++i)
    {
        adj_probs[i] = *starting_probs[i];
    }
}

state_t *pickState(trans_prob_t *tps, size_t tp_count)
{
    float sum = 0;
    float random = (float) rand() / (float) RAND_MAX;
    for (int i = 0; i < tp_count; ++i)
    {
        sum += tps[i].prob;
        if (random <= sum)
        {
            return &states[tps[i].to_id];
        }
    }
    return NULL; /* error */
}

const uint INTERACTIONS_MAX = MODEL_COUNT * MODEL_COUNT;

/* maybe namespace or encapsulate the global data storage objects
   to avoid accidental conflicts with local variables */
void run_cycle(id_t cycle_id, id_t person_id, id_t model_id)
{
    // DEBUG_PRINT("run_cycle(): cycle_id=%d person_id=%d model_id=%d\n", cycle_id, person_id, model_id);

    state_t *cur_state = person_get_state(person_id, cycle_id - 1, model_id);
    id_t *cur_state_ids = person_get_states(person_id, cycle_id - 1);
    /* always in one state for each model */
    size_t cur_states_count = MODEL_COUNT;

    trans_prob_t **cur_trans_probs = trans_probs_by_state[cur_state->id];
    size_t cur_trans_prob_count = state_get_dest_count(cur_state);

    static interaction_t *cur_interactions[INTERACTIONS_MAX];
    size_t cur_interaction_count = 0;

    static trans_prob_t *adjusted_probs = NULL;
    if (adjusted_probs == NULL)
        adjusted_probs = malloc_adjusted_probs();

    init_adjusted_probs(adjusted_probs, cur_trans_probs, cur_trans_prob_count);

    /* find any interactions */
    for (int i = 0; i < cur_states_count; ++i)
    {
        state_t *state = &states[cur_state_ids[i]];
        for (int iid = 0; iid < INTERACTION_COUNT; ++iid)
        {
            interaction_t *interaction = &interactions[iid];
            /* this condition doesn't look right */
            if (interaction->from_state_id == cur_state->id &&
                interaction->cur_state_id == state->id)
            {
                cur_interactions[cur_interaction_count] = interaction;
                cur_interaction_count += 1;
            }
        }
    }

    /* for every interaction */
    for (int i = 0; i < cur_interaction_count; ++i)
    {
        interaction_t *interaction = cur_interactions[i];
        float sum = 0.0;
        /* for every transition probablility */
        for (int i = 0; i < cur_trans_prob_count; ++i)
        {
            trans_prob_t *trans_prob = &adjusted_probs[i];
            if (trans_prob->from_id == interaction->from_state_id &&
                trans_prob->to_id == interaction->to_state_id)
            {
                trans_prob->prob *= interaction->adjustment;
                sum += trans_prob->prob;
            }
        }
        float new_adj_factor = 1.0 / sum;
        /* for every transition probablility */
        for (int i = 0; i < cur_trans_prob_count; ++i)
        {
            adjusted_probs[i].prob *= new_adj_factor;
        }
    }

    state_t *new_state = pickState(adjusted_probs, cur_trans_prob_count);
    person_add_state(person_id, cycle_id, new_state);
}

/* used for random access to models */
uint model_indices[MODEL_COUNT];

void run_model(void)
{
    /* init model_indices */
    for (int i = 0; i < MODEL_COUNT; ++i)
        model_indices[i] = i;

    /* for every cycle, starting at 1 (first real cycle) */
    for (int cid = 1; cid < CYCLE_COUNT; ++cid)
    {
        DEBUG_PRINT("run_model(): cycle %d start\n", cid);
        /* for every person */
        for (int pid = 0; pid < PERSON_COUNT; ++pid)
        {
            /* for every model, in random order */
            // suffle(model_indices, MODEL_COUNT);
            for (int i = 0; i < MODEL_COUNT; ++i)
            {
                id_t mid = model_indices[i];
                run_cycle(cid, pid, mid);
            }
        }
    }
}

void dumpCSVs(void)
{
    mkdir("tmp", ACCESSPERMS);
    FILE *f = fopen("tmp/masters.csv", "w");

    if (f == NULL) {
        printf("dumpCSVs(): fopen(): %s\n", strerror(errno));
        exit(1);
    }

    fprintf(f, "cycle_id,person_id,state_id,model_id\n");

    for (int i = 0; i < master_count; ++i)
    {
        fprintf(f, "%d,%d,%d,%d\n",
            masters[i].cycle_id,
            masters[i].person_id,
            masters[i].state_id,
            masters[i].model_id
        );
    }
    fclose(f);
}

int main(int argc, char const *argv[])
{
    clock_t start, end;

    srand(time(NULL));

    printf("init ...\n");
    start = clock();

    init_indexes();
    init_models();
    init_masters();
    init_world();

    printf("modelling ...\n");

    run_model();

    end = clock();

    printf("done\n");
    printf("elapsed time: %.9fs\n", (end - start)/1000000.0);
    printf("writing results ...\n");

    dumpCSVs();

    return 0;
}
