# 2021 Fall 黃俊穎 網程設作業

Recommended compile & run on Linux system.

---

- [HW1 BBS Server](HW1)
  - 簡易 BBS server, 支援同時多人在線、發送訊息、接收訊息
  - 進度: 已完成 multithread (100%)
  - 編譯: `make`
  - 開啟 Server : `./hw1 <port>`
  - Client 連接 : `nc <IP> <port>`
  - [指令&範例](https://hackmd.io/@Cw77l7QTQyuEGh-tVg3fjA/SyfBvXd4t)
  - `感謝 1am9trash 指導 multithread 語法`

- [HW2 Also BBS Server](HW2)
  - 可以創版，在板上發帖，可以回覆帖子
  - 進度: 已完成 (100%)
  - 編譯: `make`
  - 開啟 Server : `./hw2 <port>`
  - Client 連接 : `nc <IP> <port>`
  - 也可以使用 client.cpp 開啟 Client : `g++ client.cpp -o client`, `client <IP> <port>`S
  - [指令&範例](https://hackmd.io/XTPsGsJBT3KH8NBgW91wLA)