#pragma once

#include "CoreTypes.h"
#include "ObjectTrace.h"
#include "CoreMinimal.h"
#include "Trace/Trace.h"

#if !defined(BAND_TRACE_ENABLED)
#if UE_TRACE_ENABLED && !UE_BUILD_SHIPPING
#define BAND_TRACE_ENABLED 1
#else
#define BAND_TRACE_ENABLED 0
#endif
#endif

#if BAND_TRACE_ENABLED

UE_TRACE_CHANNEL_EXTERN(BandChannel, BAND_API)

// Emits trace events denoting scope/lifetime of an activity on the cooking channel.
#define UE_SCOPED_BANDTIMER(name)						TRACE_CPUPROFILER_EVENT_SCOPE_ON_CHANNEL(name, BandChannel)
#define UE_SCOPED_BANDTIMER_AND_DURATION(name, durationStorage) \
FScopedDurationTimer name##Timer(durationStorage); UE_SCOPED_COOKTIMER(name)

#else

#define UE_SCOPED_BANDTIMER(name)
#define UE_SCOPED_BANDTIMER_AND_DURATION(name, durationStorage)

#endif // BAND_TRACE_ENABLED