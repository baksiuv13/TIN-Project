# TIN
# Bartłomiej Kulik

"""Module responsible for receiving data."""

import queue
import select
import struct
import threading

# bytes
_RECV_PORTION_SIZE = 4
_RECV_QUEUE_SIZE = 32 * 1024

# seconds
GET_BYTE_TIMEOUT_SEC = 60
RECV_PUT_BYTE_TIMEOUT_SEC = 1

class Receiver(threading.Thread):
    """Class responsible for receiving data."""

    def __init__(self, socket, recv_read_pipe, send_write_pipe):
        """Prepare receiver resources."""
        threading.Thread.__init__(self)
        self._s = socket
        self._read_bytes_queue = queue.Queue(_RECV_QUEUE_SIZE)
        self._recv_read_pipe = recv_read_pipe
        self._send_write_pipe = send_write_pipe

    def run(self):
        """Receive data and check if given pipe is not available."""
        while True:
            avaible_read_sources, _, _ = select.select(
                [self._s, self._recv_read_pipe],
                [],
                [])

            if self._recv_read_pipe in avaible_read_sources:
                # pipe - interrupt
                print('Receiver: pipe - interrupt.')
                self._send_write_pipe.close()
                return
            if self._s in avaible_read_sources:
                data = self._s.recv(_RECV_PORTION_SIZE)
                if data:
                    for byte in data:
                        # the only place with a put operation
                        try:
                            self._read_bytes_queue.put(
                                byte,
                                block=True,
                                timeout=RECV_PUT_BYTE_TIMEOUT_SEC)
                        except queue.Full:
                            # full queue - try send again in next loop
                            # maybe _error_read_pipe is available?
                            pass
                else: # not data
                    # Connection has been lost!
                    print('Receiver: Connection has been lost!')
                    self._send_write_pipe.close()
                    return


    def _get_byte(self):
        """
        Return one byte from receiver.

        It blocks at most GET_BYTE_TIMEOUT_SEC and raises the queue.Empty
        exception if no item was available within that time.
        """
        return self._read_bytes_queue.get(block=True,
                                          timeout=GET_BYTE_TIMEOUT_SEC)

    def _get_byte_array(self, size):
        """
        Return byte array from receiver.

        It blocks at most GET_BYTE_TIMEOUT_SEC and raises the queue.Empty
        exception if no item was available within that time.
        """
        byte_array = []
        for _ in range(size):
            byte_array.append(self._get_byte())
        return byte_array

    def get_string_value(self, size):
        """
        Return string_value from receiver.

        It blocks at most GET_BYTE_TIMEOUT_SEC and raises the queue.Empty
        exception if no item was available within that time.
        """
        byte_array = self._get_byte_array(size)
        string_value = ''
        for byte in byte_array:
            string_value += chr(byte)

        return string_value

    def get_int32_value(self):
        """
        Return int32_value from receiver.

        It blocks at most GET_BYTE_TIMEOUT_SEC and raises the queue.Empty
        exception if no item was available within that time.
        """
        byte_array = self._get_byte_array(4)
        int32_value = struct.unpack('!i', bytes(byte_array))

        return int32_value[0]

    def get_unsigned_char8_value(self):
        """
        Return unsigned_char8_value from receiver.

        It blocks at most GET_BYTE_TIMEOUT_SEC and raises the queue.Empty
        exception if no item was available within that time.
        """
        byte_array = self._get_byte_array(1)
        unsigned_char8_value = struct.unpack('!B', bytes(byte_array))

        return unsigned_char8_value[0]

    def get_double64_value(self):
        """
        Return double64_value from receiver.

        It blocks at most GET_BYTE_TIMEOUT_SEC and raises the queue.Empty
        exception if no item was available within that time.
        """
        byte_array = self._get_byte_array(8)
        double64_value = struct.unpack('!d', bytes(byte_array))

        return double64_value[0]
