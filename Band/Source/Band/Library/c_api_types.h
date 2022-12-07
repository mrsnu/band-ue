#ifndef BAND_C_C_API_TYPE_H_
#define BAND_C_C_API_TYPE_H_

typedef enum BandConfigField {
  BAND_PROFILE_ONLINE = 0,
  BAND_PROFILE_NUM_WARMUPS = 1,
  BAND_PROFILE_NUM_RUNS = 2,
  BAND_PROFILE_COPY_COMPUTATION_RATIO = 3,
  BAND_PROFILE_SMOOTHING_FACTOR = 4,
  BAND_PROFILE_DATA_PATH = 5,
  BAND_PLANNER_SCHEDULE_WINDOW_SIZE = 6,
  BAND_PLANNER_SCHEDULERS = 7,
  BAND_PLANNER_CPU_MASK = 8,
  BAND_PLANNER_LOG_PATH = 9,
  BAND_WORKER_WORKERS = 10,
  BAND_WORKER_CPU_MASKS = 11,
  BAND_WORKER_NUM_THREADS = 12,
  BAND_WORKER_ALLOW_WORKSTEAL = 13,
  BAND_WORKER_AVAILABILITY_CHECK_INTERVAL_MS = 14,
  BAND_MODEL_MODELS = 15,
  BAND_MODEL_PERIODS = 16,
  BAND_MODEL_BATCH_SIZES = 17,
  BAND_MODEL_ASSIGNED_WORKERS = 18,
  BAND_MODEL_SLOS_US = 19,
  BAND_MODEL_SLOS_SCALE = 20,
  BAND_MINIMUM_SUBGRAPH_SIZE = 21,
  BAND_SUBGRAPH_PREPARATION_TYPE = 22,
  BAND_CPU_MASK = 23,
} BandConfigField;

#endif