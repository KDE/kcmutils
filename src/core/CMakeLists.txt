add_library(KF5KCMUtilsCore STATIC)

target_sources(KF5KCMUtilsCore PRIVATE kpluginmodel.cpp)
target_link_libraries(KF5KCMUtilsCore
  PUBLIC
    KF5::CoreAddons
  PRIVATE
    KF5::Service # KServiceTypeTrader compat code path
    KF5::ItemViews
)
