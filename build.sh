mkdir -p build || exit
cd build || exit
cmake .. || exit
cd .. || exit
cmake --build build --target parser_generator || exit