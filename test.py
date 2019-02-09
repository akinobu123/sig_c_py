import time
import os
import sys
import signal
import argparse

parser = argparse.ArgumentParser(description='pix2pix')
parser.add_argument('--parent_pid', type=int, default=0)
parser.add_argument('--prefix', default=None)
args = parser.parse_args()

# ----------------- pix2pixの実行ハンドラ ------------------
def exec_pix2pix(signum, frame):
	print("py : receive SIGUSR1");
	print("py : pix2pix start")
	time.sleep(2)
	print("py : pix2pix end")
	print("py -> c : SIGUSR1");
	os.kill(args.parent_pid, signal.SIGUSR1)		# 終了のシグナルを返送

# ----------------- スタンバイモード解除＆終了 -------------------
def set_exit(signum, frame):
	print("py : receive SIGUSR2");
	print("py : python end")
	sys.exit()

# ----------------- PIDファイル作成しスタンバイモードへ ----------------
# PIDファイル作成
pid = os.getpid()
filename = '{}{}'.format(args.prefix, pid)
f = open(filename, 'w')
f.close()

# シグナルを受け取る準備をする
signal.signal(signal.SIGUSR1, exec_pix2pix)
signal.signal(signal.SIGUSR2, set_exit)

# スタンバイモードへ
while True:
	print('py : waiting...')
	signal.pause()

