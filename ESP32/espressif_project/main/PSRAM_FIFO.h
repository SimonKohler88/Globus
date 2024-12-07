/*
 * PSRAM_FIFO.h
 *
 *  Created on: 19.10.2024
 *      Author: skohl
 */

#ifndef MAIN_PSRAM_FIFO_H_
#define MAIN_PSRAM_FIFO_H_

#include "hw_settings.h"
#include "stdint.h"

struct
{
    uint8_t ready_4_fpga_frames;
    uint8_t free_frames;
    uint8_t current_frame_2_esp;
    uint8_t current_frame_2_fpga;
} typedef fifo_status_t;

struct
{
    uint8_t* frame_start_ptr;
    volatile uint8_t* current_ptr;
    volatile uint32_t size;
    uint32_t total_size;
} typedef fifo_frame_t;

/**
 * @brief Initializes the FIFO system for managing frames by allocating memory and setting up queues.
 *
 * This function sets up the FIFO control system by linking the provided status structure,
 * calculating the frame size, and creating FreeRTOS queues for managing frames. Memory is
 * allocated for each frame from the PSRAM, and the static frame buffer is initialized.
 * Also logs the allocation status for each frame.
 *
 * @param status Pointer to a fifo_status_t structure that will contain FIFO status information.
 */
void fifo_init( fifo_status_t* status );

/**
 * Checks the number of frames available in the FIFO queue that are ready for FPGA processing.
 *
 * @return The number of frames currently waiting in the `ready_4_fpga_frames` queue.
 */
uint8_t fifo_has_frame_4_fpga( void );

/**
 * Retrieves a frame from the FIFO queue that is ready to be processed by the FPGA.
 *
 * This function checks if there is a frame currently in progress to be sent to the FPGA.
 * If there is no frame currently in progress, it attempts to fetch a frame from the queue
 * of frames ready for the FPGA. If a frame is successfully retrieved, it updates the internal
 * state indicating that a frame is in progress and returns a pointer to the frame.
 *
 * @return A pointer to the frame ready for FPGA if successful, or NULL if no frame is ready
 *         or another frame is already in progress.
 */
fifo_frame_t* fifo_get_frame_4_fpga( void );

/**
 * Marks the frame for the FPGA as completed and resets the current frame state.
 *
 * This function is responsible for marking the current frame being sent to the
 * FPGA as completed. It resets the current frame pointers and size, sends the
 * current frame back to the queue of free frames, and updates internal statistics.
 * The operation occurs only if a frame is currently in progress.
 *
 * Preconditions:
 * - The frame to FPGA transfer must be in progress.
 *
 * Postconditions:
 * - The current frame is reset and returned to the free frames queue.
 * - Internal statistics are updated.
 */
void fifo_mark_frame_4_fpga_done( void );

/**
 * Checks if a frame transfer to the FPGA is currently in progress.
 *
 * @return A non-zero value if a frame transfer to the FPGA is in progress;
 *         zero if no transfer is in progress.
 */
uint8_t fifo_is_frame_2_fpga_in_progress( void );

/**
 * Checks if there are any free frames available in the FIFO.
 *
 * This function determines the number of free frame slots currently
 * available in the FIFO by checking the messages waiting in the free_frames queue.
 *
 * @return The number of free frames available in the FIFO.
 */
uint8_t fifo_has_free_frame( void );

/**
 * Retrieves a free frame from the FIFO queue that is available for processing.
 *
 * This function checks if a frame transfer from RPi to FIFO is currently in progress.
 * If a transfer is in progress, it returns NULL immediately, indicating no frames are
 * available for processing. If a transfer is not in progress, it attempts to receive a
 * frame from the free frames queue. If a frame is successfully received, it updates
 * the FIFO control status, sets the frame transfer from RPi to FIFO flag, and returns
 * a pointer to the current frame for processing. If no frame is available, it returns
 * NULL.
 *
 * @return Pointer to the current frame from RPi if available, otherwise NULL.
 */
fifo_frame_t* fifo_get_free_frame( void );

/**
 * @brief Marks the current frame being transferred from the Raspberry Pi (RPI) to the FIFO as done.
 *
 * This function is called when a frame transfer from the Raspberry Pi to the FIFO is completed.
 * It moves the current frame to the queue for frames ready to be processed by the FPGA, updates
 * the status of the transfer in progress to not be in progress, and updates the FIFO usage statistics.
 *
 * The function checks if a frame transfer from the Raspberry Pi to the FIFO is in progress.
 * If a transfer is in progress, it sends the completed frame to the queue `ready_4_fpga_frames`
 * and resets the flag `frame_rpi_2_fifo_in_progress`. After marking the frame as done, it updates
 * the statistics for FIFO operations.
 *
 * This function logs the completion of a frame transfer and is expected to be run in a context
 * where the conditions for frame completion are satisfied to maintain accurate statistics and
 * orderly processing of frames.
 *
 * @note This function assumes that there is an ongoing frame transfer and is designed to be
 * called after receiving the last packet of a frame. It is typically used in the context of
 * processing frames received over a network.
 */
void fifo_mark_free_frame_done( void );

/**
 * @brief Returns the current frame from RPI back to the free frames queue.
 *
 * This function sends the current frame received from the Raspberry Pi (RPI)
 * back to the queue of free frames, making it available for reuse. It also
 * updates the status indicating that no frame is in progress of being transferred
 * from RPI to the FIFO. The function subsequently calls `fifo_update_stats` to
 * update the FIFO status information.
 *
 * The function should be called when the processing of a frame from RPI has
 * been completed or needs to be aborted and the frame needs to be returned to
 * the pool of available frames.
 *
 * @note This function assumes that the queue `fifo_control.free_frames` is
 * properly initialized and that there is an ongoing frame transaction indicated
 * by the `fifo_control.frame_rpi_2_fifo_in_progress` flag.
 */
void fifo_return_free_frame( void );

/**
 * @brief Updates the FIFO system's status.
 *
 * This function refreshes the FIFO control status structure with the current
 * statistics of the FIFO operations. It updates the number of free frames,
 * frames ready for FPGA, and flags indicating whether frames are currently
 * being transferred to FPGA or from Raspberry Pi to FIFO. This is useful
 * for monitoring or debugging the FIFO operations.
 *
 * @note This function should be called after any operation that affects
 * the FIFO queues, such as retrieving or returning frames.
 */
void fifo_update_stats( void );

/**
 * Retrieves a pointer to the static picture frame stored in memory.
 *
 * This function returns a pointer to a pre-defined static `fifo_frame_t`
 * structure, which is utilized for managing and accessing a frame data
 * stored in memory. It is particularly useful for cases where a static frame
 * is required for consistent and repeated access, such as in testing scenarios
 * or where a common reference frame is necessary.
 *
 * @return A pointer to the `fifo_frame_t` structure representing the static
 *         picture frame.
 */
fifo_frame_t* fifo_get_static_frame( void );

/**
 * @brief Copies memory from a source to a destination with mutual exclusion to ensure thread safety.
 *
 * This function copies a block of memory from the specified source pointer to the
 * destination pointer. It uses a mutex to protect the memory operation, ensuring
 * that the memory copy is thread-safe and preventing potential data corruption
 * during simultaneous access.
 *
 * @param dst_ptr Pointer to the destination memory where the data is to be copied.
 * @param src_ptr Pointer to the source memory from which the data is to be copied.
 * @param size Size, in bytes, of the data to be copied from the source to the destination.
 */
void fifo_copy_mem_protected( void* dst_ptr, const void* src_ptr, uint32_t size );

#endif /* MAIN_PSRAM_FIFO_H_ */
