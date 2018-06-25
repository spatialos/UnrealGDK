// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_METRICS_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_METRICS_I_H
#include <improbable/worker.h>
#include <limits>

namespace worker {

inline HistogramMetric::HistogramMetric(const List<double>& bucket_boundaries) : sum{0} {
  static constexpr double infinity = std::numeric_limits<double>::infinity();
  bool has_infinity = false;
  for (double le : bucket_boundaries) {
    has_infinity |= le == infinity;
    buckets.emplace_back(Bucket{le, 0});
  }
  if (!has_infinity) {
    buckets.emplace_back(Bucket{infinity, 0});
  }
}

inline HistogramMetric::HistogramMetric() : HistogramMetric(List<double>{}) {}

inline void HistogramMetric::ClearObservations() {
  for (auto& bucket : buckets) {
    bucket.Samples = 0;
  }
  sum = 0;
}

inline void HistogramMetric::RecordObservation(double value) {
  for (auto& bucket : buckets) {
    if (value <= bucket.UpperBound) {
      ++bucket.Samples;
    }
  }
  sum += value;
}

inline const List<HistogramMetric::Bucket>& HistogramMetric::Buckets() const {
  return buckets;
}

inline double HistogramMetric::Sum() const {
  return sum;
}

inline void Metrics::Merge(const Metrics& metrics) {
  if (metrics.Load) {
    Load = metrics.Load;
  }
  for (const auto& pair : metrics.GaugeMetrics) {
    GaugeMetrics[pair.first] = pair.second;
  }
  for (const auto& pair : metrics.HistogramMetrics) {
    HistogramMetrics[pair.first] = pair.second;
  }
}

}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_METRICS_I_H
