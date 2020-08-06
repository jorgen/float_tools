set(Fetch3rdPartyDir ${CMAKE_CURRENT_LIST_DIR})

macro(Fetch3rdParty_Package name version url url_hash)
    get_filename_component(thirdParty "${Fetch3rdPartyDir}/../3rdparty" ABSOLUTE)
    set(SRC_DIR ${thirdParty}/${name}-${version})
    set(${name}_SOURCE_DIR ${SRC_DIR} PARENT_SCOPE)
    set(${name}_VERSION ${version} PARENT_SCOPE)
  if (NOT (EXISTS ${SRC_DIR}))
    FetchContent_Populate(${name}
      URL ${url}
      URL_HASH ${url_hash}
      SOURCE_DIR ${SRC_DIR}
      SUBBUILD_DIR ${thirdParty}/CMakeArtifacts/${name}-sub-${version}
      BINARY_DIR ${thirdParty}/CMakeArtifacts/${name}-${version})
  endif()
endmacro()


function(Fetch3rdParty)
  include(FetchContent)
  set(FETCHCONTENT_QUIET OFF)

  Fetch3rdParty_Package(fmt 6.0.0 https://github.com/fmtlib/fmt/archive/6.0.0.tar.gz SHA256=f1907a58d5e86e6c382e51441d92ad9e23aea63827ba47fd647eacc0d3a16c78)
endfunction()

