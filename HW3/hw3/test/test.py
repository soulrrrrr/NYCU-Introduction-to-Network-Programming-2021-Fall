#!/usr/bin/python3

import subprocess
import os
import re
import libtmux
from time import sleep
import sys

start_dir = '/project/test'


class TEST_TOOL:
    def __init__(self, server) -> None:
        self.server = server
        self.point = 0
        self.port = 8887


    class DATA:
        def get_server_cmd(self):
            self.port += 1
            return f'./hw3 {self.port}'

        def get_client_cmd(self):
            return f'python3 client.py {self.port}'

        BASIC_CASES = ['1-register', '2-login', '3-logout', '4-exit',
                       '5-enter-chat-room', '6-chat', '7-filter-and-blacklist']
        BASIC_POINTS = [5, 5, 5, 5, 10, 10, 10]
        BASIC_FOLDER = './basic'
        BASIC_TITLE = 'BASIC TEST CASES'

        # ADVANCE_CASES = ['8-advance-case1', '9-advance-case2', '10-advance-case3', '11-advance-case4', '12-advance-case5']
        ADVANCE_CASES = []
        # ADVANCE_POINTS = [10, 10, 10, 10, 10]
        ADVANCE_POINTS = []
        ADVANCE_FOLDER = './advance'
        ADVANCE_TITLE = 'ADVANCE TEST CASES'

        TOTAL_POINTS = sum(BASIC_POINTS) + sum(ADVANCE_POINTS)

    class C:
        RED = "\x1b[38;5;1m"
        GREEN = "\x1b[38;5;2m"
        YELLOW = "\x1b[38;5;3m"
        BLUE = "\x1b[38;5;4m"
        GRAY = "\x1b[38;5;8m"
        CLEAR = "\x1b[0m"

    def _get_port(self):
        return self.port

    def _log(self, *content, sign='+', log_color=C.BLUE):
        print(f'[{log_color}{sign}{self.C.CLEAR}]', *content)

    def _err_log(self, info):
        sys.stderr.write(f'{self.C.RED}[!] {info}{self.C.CLEAR}\n')

    def _reset_and_get_window(self):
        # Kill all sessions
        if 'no server running' not in os.popen('tmux ls').read():
            os.system('tmux kill-session')
        self.server.new_session(session_name='test-session', start_directory=start_dir)
        session = self.server.list_sessions()[0]
        return session.attached_window

    def _send_command(self, pane, command: str):
        pane.send_keys(command)
        sleep(0.03)

    def _test_case_runner(self, path: str, out_path: str):
        data = open(path, 'r').readlines()
        if len(data) < 1:
            self._err_log(f"Client number not found in '{path}' !")
            exit(1)

        client_cnt = int(data[0])
        if client_cnt < 1:
            self._err_log(f"Client number should be more that 0 !")
            exit(1)

        window = self._reset_and_get_window()
        sleep(0.1)

        # Generate panes
        for _ in range(client_cnt):
            window.split_window(attach=False, start_directory=start_dir, vertical=False)
            window.select_layout('tiled')
        window.select_layout('tiled')

        # Start server and client
        panes = window.list_panes()
        self._send_command(panes[0], self.DATA.get_server_cmd(self))
        self._send_command(panes[1], f'{self.DATA.get_client_cmd(self)} > {out_path}')
        for pane in panes[2:]:
            self._send_command(pane, self.DATA.get_client_cmd(self))

        # Parse and send commands
        for el in data[1:]:
            d = re.findall(r'^(\d+)\s(.+)$', el)[0]
            pane = panes[int(d[0])]
            command = d[1]
            self._send_command(pane, command)

    def _diff_check(self, output: str, solution: str):
        res = os.popen(f'diff {output} {solution}').read()
        open(f'{output}.diff', 'w').write(res)
        return res == ''

    def _gen_final_grade(self):
        self._gen_horizontal_line()
        color = self.C.GREEN if self.point/self.DATA.TOTAL_POINTS > 0.99 else self.C.RED
        self._log(f"{'Final grade'.ljust(30)}{color}{self.point}/{self.DATA.TOTAL_POINTS}{self.C.CLEAR}",
                  sign='x', log_color=self.C.GREEN)

    def _gen_horizontal_line(self):
        self._log('-'*38, sign='-', log_color=self.C.CLEAR)

    def _start_test(self, title: str, folder: str, cases: str, points: int):
        self._gen_horizontal_line()
        self._log(title, sign='-', log_color=self.C.CLEAR)
        self._gen_horizontal_line()

        # Traverse all testcases
        for file_name, point in zip(cases, points):
            test_case_path = f"{folder}/test-cases/{file_name}"
            solution_path = f"{folder}/solution/{file_name}"
            output_path = f"{folder}/output/{file_name}"

            # Execute `client` and store the output from 1st client
            self._test_case_runner(test_case_path, output_path)

            # Compare with the `diff` command
            result = self._diff_check(output_path, solution_path)
            p = point * result
            color = self.C.GREEN if self._diff_check(output_path, solution_path) else self.C.RED
            self._log(f'{file_name.ljust(30)}{color}{p}/{point}{self.C.CLEAR}')
            self.point += p


    def run(self):
        self._start_test(self.DATA.BASIC_TITLE, self.DATA.BASIC_FOLDER, self.DATA.BASIC_CASES, self.DATA.BASIC_POINTS)
        self._start_test(self.DATA.ADVANCE_TITLE, self.DATA.ADVANCE_FOLDER,
                         self.DATA.ADVANCE_CASES, self.DATA.ADVANCE_POINTS)
        self._gen_final_grade()
        self._gen_horizontal_line()


def main():
    server = libtmux.Server()

    test_tool = TEST_TOOL(server)
    test_tool.run()


if __name__ == '__main__':
    print()
    main()
    print()
