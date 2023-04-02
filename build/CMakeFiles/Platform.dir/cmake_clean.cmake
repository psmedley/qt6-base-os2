file(REMOVE_RECURSE
  "lib/pkgconfig/Qt6Platform.pc"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/Platform.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
