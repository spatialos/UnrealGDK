#ifndef GDK_METRICS_H
#define GDK_METRICS_H
#include <WorkerSDK/improbable/c_worker.h>
#include <deque>
#include <string>
#include <vector>

namespace gdk {

class Metrics {
public:
  explicit Metrics(const Worker_Metrics& metrics);

  ~Metrics() = default;

  // Moveable, not copyable
  Metrics(const Metrics&) = delete;
  Metrics(Metrics&&) = default;
  Metrics& operator=(const Metrics&) = delete;
  Metrics& operator=(Metrics&&) = default;

  const Worker_Metrics* GetMetrics() const;

private:
  void StoreHistogramMetrics(const Worker_Metrics& metrics);
  void StoreGaugeMetrics(const Worker_Metrics& metrics);

  std::deque<std::string> stringStorage;
  std::vector<Worker_HistogramMetric> histograms;
  std::vector<Worker_GaugeMetric> gauges;
  std::vector<std::vector<Worker_HistogramMetricBucket>> histogramBuckets;
  Worker_Metrics metricsRoot;
  bool hasLoad;
  double load;
};

}  // namespace gdk
#endif  // GDK_METRICS_H