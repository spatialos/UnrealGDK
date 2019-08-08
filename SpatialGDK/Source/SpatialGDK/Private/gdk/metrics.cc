#include "gdk/metrics.h"

#include <cstdint>

namespace gdk {

Metrics::Metrics(const Worker_Metrics& metrics)
: histograms(metrics.histogram_metric_count)
, gauges(metrics.gauge_metric_count)
, histogramBuckets(metrics.histogram_metric_count)
, metricsRoot()
, hasLoad(metrics.load != nullptr)
, load(hasLoad ? *metrics.load : 0.0) {
  StoreHistogramMetrics(metrics);
  StoreGaugeMetrics(metrics);

  // Save the resulting metrics object.
  metricsRoot.gauge_metric_count = static_cast<std::uint32_t>(gauges.size());
  metricsRoot.gauge_metrics = gauges.data();
  metricsRoot.histogram_metric_count = static_cast<std::uint32_t>(histograms.size());
  metricsRoot.histogram_metrics = histograms.data();
  metricsRoot.load = hasLoad ? &load : nullptr;
}

const Worker_Metrics* Metrics::GetMetrics() const {
  return &metricsRoot;
}

void Metrics::StoreHistogramMetrics(const Worker_Metrics& metrics) {
  for (std::uint32_t i = 0; i < metrics.histogram_metric_count; ++i) {
    auto& histogram = metrics.histogram_metrics[i];
    histograms[i].sum = histogram.sum;
    stringStorage.emplace_back(histogram.key);
    histograms[i].key = stringStorage.back().c_str();

    // Store and re-point histogram metrics buckets.
    histograms[i].bucket_count = histogram.bucket_count;
    histogramBuckets[i] = std::vector<Worker_HistogramMetricBucket>(histogram.bucket_count);
    for (std::uint32_t j = 0; j < histogram.bucket_count; j++) {
      histogramBuckets[i][j] = histogram.buckets[j];
    }
    histograms[i].buckets = histogramBuckets[i].data();
  }
}

void Metrics::StoreGaugeMetrics(const Worker_Metrics& metrics) {
  for (std::uint32_t i = 0; i < metrics.gauge_metric_count; ++i) {
    gauges[i].value = metrics.gauge_metrics[i].value;
    stringStorage.emplace_back(metrics.gauge_metrics[i].key);
    gauges[i].key = stringStorage.back().c_str();
  }
}

}  // namespace gdk
