import sys
import queue
import numpy as np
import serial
import serial.tools.list_ports
import pyqtgraph as pg

from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QLineEdit, QPushButton, QLabel, QComboBox
)
from PyQt6.QtCore import QThread, pyqtSignal, QTimer


class SerialReaderThread(QThread):
    connection_error = pyqtSignal(str)

    def __init__(self, port_in, baudrate_in, data_queue):
        super().__init__()
        self.port = port_in
        self.baudrate = baudrate_in
        self.data_queue = data_queue
        self._running = False
        self.raw_sample_count = 0
        self.serial_ins = None

    def run(self):
        self._running = True
        try:
            # Initialize serial port inside thread run()
            self.serial_ins = serial.Serial(port=self.port, baudrate=int(self.baudrate), timeout=1)
        except Exception as e:
            self.connection_error.emit(str(e))
            return

        print(f"Connected to {self.port} at {self.baudrate} baud.")

        while self._running:
            try:
                if self.serial_ins.in_waiting > 0:
                    raw_line = self.serial_ins.readline()
                    # Clean strings: strip whitespace, commas, carriage returns
                    line_str = raw_line.decode('utf-8', errors='ignore').strip().replace(',', '')
                    
                    if not line_str:
                        continue

                    value = float(line_str)
                    self.raw_sample_count += 1
                    
                    # Pass sample to thread-safe queue instead of flooding signals
                    self.data_queue.put(value)
            except ValueError:
                continue  # Skip header or noise lines
            except Exception as e:
                self.connection_error.emit(str(e))
                break

    def stop(self):
        self._running = False
        if self.serial_ins and self.serial_ins.is_open:
            self.serial_ins.close()
        self.wait()


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("STM32 FFT Monitor (Python-side FFT)")
        self.resize(1200, 800)

        self.sample_queue = queue.Queue()

        central = QWidget()
        self.setCentralWidget(central)
        root_layout = QVBoxLayout(central)

        # --- Connection bar ---
        conn_layout = QHBoxLayout()
        self.port_combo = QComboBox()
        ports = [p.device for p in serial.tools.list_ports.comports()]
        self.port_combo.addItems(ports)
        
        self.baud_combo = QComboBox()
        self.baud_combo.addItems(["115200", "9600", "230400", "460800"])
        self.connect_btn = QPushButton("Connect")
        
        conn_layout.addWidget(QLabel("Port:"))
        conn_layout.addWidget(self.port_combo)
        conn_layout.addWidget(QLabel("Baud:"))
        conn_layout.addWidget(self.baud_combo)
        conn_layout.addWidget(self.connect_btn)
        root_layout.addLayout(conn_layout)

        # --- FFT parameters ---
        fft_params_layout = QHBoxLayout()
        self.fft_size_input = QLineEdit("256")
        self.fft_size_input.setFixedWidth(80)
        self.sample_rate_input = QLineEdit("1000")
        self.sample_rate_input.setFixedWidth(80)
        
        fft_params_layout.addWidget(QLabel("FFT Size:"))
        fft_params_layout.addWidget(self.fft_size_input)
        fft_params_layout.addWidget(QLabel("Sample Rate (Hz):"))
        fft_params_layout.addWidget(self.sample_rate_input)
        fft_params_layout.addStretch()
        root_layout.addLayout(fft_params_layout)

        # --- Rate diagnostics ---
        rate_layout = QHBoxLayout()
        self.uart_rate_label = QLabel("UART samples/sec: 0")
        self.processed_rate_label = QLabel("Processed/sec: 0")
        rate_layout.addWidget(self.uart_rate_label)
        rate_layout.addWidget(self.processed_rate_label)
        rate_layout.addStretch()
        root_layout.addLayout(rate_layout)

        # --- Plot Widget ---
        self.plot_widget = pg.PlotWidget()
        self.plot_widget.setLabel('left', 'Magnitude')
        self.plot_widget.setLabel('bottom', 'Frequency (Hz)')
        self.plot_widget.getPlotItem().getAxis('left').enableAutoSIPrefix(False)
        self.spectrum_curve = self.plot_widget.plot(pen=pg.mkPen('c', width=2))
        root_layout.addWidget(self.plot_widget)

        # --- Timers ---
        # 1. High-frequency timer to drain serial queue & update plot (~30 FPS)
        self._plot_timer = QTimer(self)
        self._plot_timer.setInterval(33)  # ~30 Hz refresh rate
        self._plot_timer.timeout.connect(self._process_queue_and_plot)

        # 2. Diagnostics rate timer
        self._last_raw_count = 0
        self._processed_count = 0
        self._rate_timer = QTimer(self)
        self._rate_timer.setInterval(1000)
        self._rate_timer.timeout.connect(self._update_rate_labels)
        self._rate_timer.start()

        self._window_buffer = []
        self.reader_thread = None
        self.connect_btn.clicked.connect(self.on_connect_clicked)

    def on_connect_clicked(self):
        if self.reader_thread and self.reader_thread.isRunning():
            self.reader_thread.stop()
            self.connect_btn.setText("Connect")
            self._plot_timer.stop()
            return

        port = self.port_combo.currentText()
        baudrate = self.baud_combo.currentText()

        if not port:
            return

        self._window_buffer = []
        self._last_raw_count = 0
        self._processed_count = 0
        
        # Clear queue
        while not self.sample_queue.empty():
            try:
                self.sample_queue.get_nowait()
            except queue.Empty:
                break

        self.reader_thread = SerialReaderThread(port, baudrate, self.sample_queue)
        self.reader_thread.connection_error.connect(self.on_connection_error)
        self.reader_thread.start()
        
        self.connect_btn.setText("Disconnect")
        self._plot_timer.start()

    def on_connection_error(self, err):
        print(f"Serial Error: {err}")
        self.on_connect_clicked()

    def _process_queue_and_plot(self):
        # Drain queue into buffer
        samples_added = 0
        while not self.sample_queue.empty():
            try:
                val = self.sample_queue.get_nowait()
                self._window_buffer.append(val)
                samples_added += 1
            except queue.Empty:
                break

        self._processed_count += samples_added

        try:
            fft_size = int(self.fft_size_input.text())
        except ValueError:
            fft_size = 256

        # Check if enough samples for an FFT frame
        if len(self._window_buffer) >= fft_size:
            # Get latest frame window
            window_data = np.array(self._window_buffer[-fft_size:])
            
            # Keep buffer bounded to prevent memory leaks
            if len(self._window_buffer) > fft_size * 4:
                self._window_buffer = self._window_buffer[-fft_size:]

            self._compute_and_plot_fft(window_data, fft_size)

    def _compute_and_plot_fft(self, window_data, fft_size):
        try:
            sample_rate = float(self.sample_rate_input.text())
        except ValueError:
            sample_rate = 1000.0

        # Apply Hanning window to eliminate spectral leakage
        hanning_window = window_data * np.hanning(len(window_data))

        # Real FFT magnitude calculation
        fft_result = np.fft.rfft(hanning_window)
        magnitude = np.abs(fft_result) / (fft_size / 2.0)
        freqs = np.fft.rfftfreq(fft_size, d=1.0 / sample_rate)

        # Update curve line efficiently
        self.spectrum_curve.setData(freqs, magnitude)

    def _update_rate_labels(self):
        raw_count = self.reader_thread.raw_sample_count if (self.reader_thread and self.reader_thread.isRunning()) else 0
        uart_rate = raw_count - self._last_raw_count
        self._last_raw_count = raw_count
        
        self.uart_rate_label.setText(f"UART samples/sec: {uart_rate}")
        self.processed_rate_label.setText(f"Processed/sec: {self._processed_count}")
        self._processed_count = 0


def main():
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()