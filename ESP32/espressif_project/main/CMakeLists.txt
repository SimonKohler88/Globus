# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS main.c jpeg2raw.c tjpgd/tjpgd.c jpeg_decoder.c jpeg_default_huffman_table.c psram_fifo.c pic_buffer.c wifi.c http_task.c qspi.c rotor_encoding.c status_control_task.c status_control_task_helper.c psram_fifo_static_pic.c# list the source files of this component
    INCLUDE_DIRS        tjpgd # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
