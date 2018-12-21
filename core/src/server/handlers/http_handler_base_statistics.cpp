#include <server/handlers/http_handler_base_statistics.hpp>

namespace server {
namespace handlers {

HttpHandlerMethodStatistics& HttpHandlerStatistics::GetStatisticByMethod(
    http::HttpMethod method) {
  size_t index = static_cast<size_t>(method);
  assert(index < stats_by_method_.size());

  return stats_by_method_[index];
}

const HttpHandlerMethodStatistics& HttpHandlerStatistics::GetStatisticByMethod(
    http::HttpMethod method) const {
  size_t index = static_cast<size_t>(method);
  assert(index < stats_by_method_.size());

  return stats_by_method_[index];
}

HttpHandlerMethodStatistics& HttpHandlerStatistics::GetTotalStatistics() {
  return stats_;
}

const HttpHandlerMethodStatistics& HttpHandlerStatistics::GetTotalStatistics()
    const {
  return stats_;
}

bool HttpHandlerStatistics::IsOkMethod(http::HttpMethod method) const {
  return static_cast<size_t>(method) < stats_by_method_.size();
}

void HttpHandlerStatistics::Account(http::HttpMethod method, unsigned int code,
                                    std::chrono::milliseconds ms) {
  GetTotalStatistics().Account(code, ms.count());
  if (IsOkMethod(method))
    GetStatisticByMethod(method).Account(code, ms.count());
}

HttpHandlerStatisticsScope::HttpHandlerStatisticsScope(
    HttpHandlerStatistics& stats, http::HttpMethod method)
    : stats_(stats), method_(method) {
  stats_.GetTotalStatistics().IncrementInFlight();
  if (stats_.IsOkMethod(method))
    stats_.GetStatisticByMethod(method).IncrementInFlight();
}

void HttpHandlerStatisticsScope::Account(unsigned int code,
                                         std::chrono::milliseconds ms) {
  stats_.Account(method_, code, ms);

  stats_.GetTotalStatistics().DecrementInFlight();
  if (stats_.IsOkMethod(method_))
    stats_.GetStatisticByMethod(method_).DecrementInFlight();
}

}  // namespace handlers
}  // namespace server
