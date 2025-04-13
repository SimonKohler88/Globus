# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS main.c  misc_task.c pic_buffer.c wifi.c http_task.c rpi_interface.c qspi.c rotor_encoding.c status_control_task.c fpga_ctrl_task.c status_control_task_helper.c psram_fifo_static_pic.c# list the source files of this component
    INCLUDE_DIRS        # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)

# add_subdirectory(videoplayer)