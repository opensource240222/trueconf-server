
#ifndef HIDAPI_H__
#define HIDAPI_H__

#include <wchar.h>
#include <windows.h>

struct hid_device {
	HANDLE device_handle;
	BOOL blocking;
	USHORT output_report_length;
	size_t input_report_length;
	void *last_error_str;
	DWORD last_error_num;
	BOOL read_pending;
	char *read_buf;
	OVERLAPPED ol;
};
/** hidapi info structure */
struct hid_device_info {
	/** Platform-specific device path */
	char *path;
	/** Device Vendor ID */
	unsigned short vendor_id;
	/** Device Product ID */
	unsigned short product_id;
	/** Serial Number */
	wchar_t *serial_number;
	/** Device Release Number in binary-coded decimal,
	also known as Device Version Number */
	unsigned short release_number;
	/** Manufacturer String */
	wchar_t *manufacturer_string;
	/** Product string */
	wchar_t *product_string;
	/** Usage Page for this Device/Interface
	(Windows/Mac only). */
	unsigned short usage_page;
	/** Usage for this Device/Interface
	(Windows/Mac only).*/
	unsigned short usage;
	/** The USB interface which this logical device
	represents. Valid on both Linux implementations
	in all cases, and valid on the Windows implementation
	only if the device contains more than one interface. */
	int interface_number;

	/** Pointer to the next device */
	struct hid_device_info *next;
};

class HidApi {

	HidApi() {hid_init();}
	static HidApi* m_instance;
public:
	static HidApi* GetInstance();

	/** @brief Initialize the HIDAPI library.
	@returns
	This function returns 0 on success and -1 on error.
	*/
	int hid_init(void);

	/** @brief Finalize the HIDAPI library.
	@returns
	This function returns 0 on success and -1 on error.
	*/
	int hid_exit(void);

	/** @brief Enumerate the HID Devices.
	@returns
	This function returns a pointer to a linked list of type
	struct #hid_device, containing information about the HID devices
	attached to the system, or NULL in the case of failure. Free
	this linked list by calling hid_free_enumeration().
	*/
	struct hid_device_info *  hid_enumerate(unsigned short vendor_id, unsigned short product_id);

	/** @brief Free an enumeration Linked List
	*/
	void  hid_free_enumeration(struct hid_device_info *devs);

	/** @brief Open a HID device using a Vendor ID (VID), Product ID (PID) and optionally a serial number.
	@returns
	This function returns a pointer to a #hid_device object on
	success or NULL on failure.
	*/
	hid_device *  hid_open(unsigned short vendor_id, unsigned short product_id, int iface);

	/** @brief Open a HID device by its path name.
	@returns
	This function returns a pointer to a #hid_device object on
	success or NULL on failure.
	*/
	hid_device *  hid_open_path(const char *path);

	/** @brief Write an Output report to a HID device.

	hid_write() will send the data on the first OUT endpoint, if
	one exists. If it does not, it will send the data through
	the Control Endpoint (Endpoint 0).
	@returns
	This function returns the actual number of bytes written and
	-1 on error.
	*/
	int hid_write(hid_device *device, const unsigned char *data, size_t length);

	/** @brief Read an Input report from a HID device with timeout.
	@returns
	This function returns the actual number of bytes read and
	-1 on error. If no packet was available to be read within
	the timeout period, this function returns 0.
	*/
	int hid_read_timeout(hid_device *dev, unsigned char *data, size_t length, int milliseconds);

	/** @brief Read an Input report from a HID device.
	@returns
	This function returns the actual number of bytes read and
	-1 on error. If no packet was available to be read and
	the handle is in non-blocking mode, this function returns 0.
	*/
	int     hid_read(hid_device *device, unsigned char *data, size_t length);

	/** @brief Set the device handle to be non-blocking.

	In non-blocking mode calls to hid_read() will return
	immediately with a value of 0 if there is no data to be
	read. In blocking mode, hid_read() will wait (block) until
	there is data to read before returning.

	Nonblocking can be turned on and off at any time.

	@ingroup API
	@param device A device handle returned from hid_open().
	@param nonblock enable or not the nonblocking reads
	- 1 to enable nonblocking
	- 0 to disable nonblocking.

	@returns
	This function returns 0 on success and -1 on error.
	*/
	int hid_set_nonblocking(hid_device *device, int nonblock);


	/** @brief Close a HID device.
	*/
	void hid_close(hid_device *device);

	/** @brief Get The Manufacturer String from a HID device.
	@returns
	This function returns 0 on success and -1 on error.
	*/
	int hid_get_manufacturer_string(hid_device *device, wchar_t *string, size_t maxlen);

	/** @brief Get The Product String from a HID device.
	@returns
	This function returns 0 on success and -1 on error.
	*/
	int hid_get_product_string(hid_device *device, wchar_t *string, size_t maxlen);

	/** @brief Get The Serial Number String from a HID device.
	@returns
	This function returns 0 on success and -1 on error.
	*/
	int hid_get_serial_number_string(hid_device *device, wchar_t *string, size_t maxlen);

	/** @brief Get a string from a HID device, based on its string index.
	@returns
	This function returns 0 on success and -1 on error.
	*/
	int hid_get_indexed_string(hid_device *device, int string_index, wchar_t *string, size_t maxlen);

	/** @brief Get a string describing the last error which occurred.
	@returns
	This function returns a string containing the last error which occurred or NULL if none has occurred.
	*/
	const wchar_t*  hid_error(hid_device *device);
};

#endif