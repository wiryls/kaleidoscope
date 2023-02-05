# set name
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" name ${name})
string(TOLOWER ${name} name)

#[[ compile shaders ]]
# reference: https://stackoverflow.com/a/71317698

set(shader vertex_shader.hlsl pixel_shader.hlsl)
source_group("shader" FILES ${shader})
set_source_files_properties(${shader} PROPERTIES VS_TOOL_OVERRIDE "None")
set_source_files_properties(vertex_shader.hlsl PROPERTIES shader_type "vs")
set_source_files_properties(pixel_shader.hlsl  PROPERTIES shader_type "ps")

find_program(dxc dxc.exe)
foreach(file ${shader})
    get_filename_component  (file_name   ${file} NAME_WE)
    set(pdb "${CMAKE_CURRENT_BINARY_DIR}/${file_name}.pdb")
    set(out "${CMAKE_CURRENT_BINARY_DIR}/compiled_shader/${file_name}.h")
    get_source_file_property(shader_type ${file} shader_type)
    add_custom_command(OUTPUT ${out}
        COMMAND ${dxc} /nologo /Emain /T${shader_type}_6_0 $<IF:$<CONFIG:DEBUG>,/Od,/O3> /Zi /WX /Fh ${out} /Fd ${pdb} /Vn ${file_name} ${file}
        COMMENT "Compiling ${file}"
        MAIN_DEPENDENCY ${file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM)

    list(APPEND compiled_shader ${out})
endforeach(file)

#[[ executable ]]
set(source view.cc render.cc)
set(header tool.h  render.h error.h model.h viewmodel.h)

add_executable            (${name} WIN32 ${source} ${header} ${shader} ${BACKWARD_ENABLE})
add_backward              (${name})
set_target_properties     (${name} PROPERTIES FOLDER "${PROJECT_NAME}")
set_target_properties     (${name} PROPERTIES CXX_STANDARD 23)
target_include_directories(${name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/compiled_shader")
target_link_libraries     (${name} PRIVATE d3d12 d3d11 dxgi dcomp)
target_compile_options    (${name} PRIVATE
    "$<$<CXX_COMPILER_ID:MSVC>:/WX;/W4;/utf-8>"
    "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RELEASE>>:/O2>")
