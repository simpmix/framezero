# Developer Tools for FrameZero
add_executable(FrameZeroReplayViewer replay_viewer.cpp)
target_link_libraries(FrameZeroReplayViewer PRIVATE framezero)

add_executable(FrameZeroDesyncFinder desync_finder.cpp)
target_link_libraries(FrameZeroDesyncFinder PRIVATE framezero)

# If we want to install tools
install(TARGETS FrameZeroReplayViewer RUNTIME DESTINATION bin)
