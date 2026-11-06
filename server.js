// Run:  node server.js   then open http://localhost:3000
// Needs Node 16+ and g++ (or set CXX=clang++). Keep server.js, index.html and student_db.cpp in one folder.
// The C++ program is compiled from student_db.cpp on every start and driven through its normal menu
// over stdin/stdout. No logic is duplicated here: this file only relays input and reads output.
const http = require("http"),
  fs = require("fs"),
  path = require("path");
const { spawn, execFileSync } = require("child_process");
const DIR = __dirname,
  SRC = path.join(DIR, "student_db.cpp");
const BIN = path.join(
  DIR,
  process.platform === "win32" ? "student_db.exe" : "student_db",
);
const PORT = process.env.PORT || 3000;
const MENU = "\n=========== Student Database";

console.log("Compiling student_db.cpp ...");
try {
  execFileSync(
    process.env.CXX || "g++",
    ["-std=c++17", "-O2", SRC, "-o", BIN],
    { stdio: "inherit" },
  );
} catch (e) {
  console.error(
    "\nCould not compile. Install g++ (or set CXX=clang++) and try again.",
  );
  process.exit(1);
}

let child = null,
  buf = "",
  waiter = null;
const u = (s) => Buffer.from(s, "latin1").toString("utf8"); // output is read byte-for-byte, then decoded
const poke = () => waiter && waiter();

// Resolve with everything printed so far once the program is waiting at a prompt (output ends with ": ").
function read() {
  return new Promise((resolve) => {
    let t;
    const cap = setTimeout(() => finish(), 3000);
    const finish = () => {
      clearTimeout(t);
      clearTimeout(cap);
      waiter = null;
      const o = buf;
      buf = "";
      resolve(o);
    };
    waiter = () => {
      clearTimeout(t);
      if (!child || buf.endsWith(": ")) t = setTimeout(finish, 25);
    };
    waiter();
  });
}
function start() {
  const me = spawn(BIN);
  child = me;
  buf = "";
  me.stdout.setEncoding("latin1");
  me.stdout.on("data", (d) => {
    if (me === child) {
      buf += d.replace(/\r/g, "");
      poke();
    }
  });
  me.stdin.on("error", () => {});
  me.on("error", () => {});
  me.on("close", () => {
    if (me === child) {
      child = null;
      poke();
    }
  });
  return read();
}
const send = (l) => {
  child.stdin.write(l + "\n", "utf8");
  return read();
};
const cut = (s) => {
  const i = s.lastIndexOf(MENU);
  return i < 0 ? s : s.slice(0, i);
};

// Read the table printed by menu option 6 (Display all).
function parse(o) {
  const records = [];
  for (const ln of o.split("\n")) {
    const roll = ln.slice(0, 8).trim();
    if (!/^\d+$/.test(roll) || ln.length < 41) continue;
    records.push({
      roll: +roll,
      name: u(ln.slice(8, 30).trim()),
      branch: u(ln.slice(30, 40).trim()),
      cgpa: parseFloat(ln.slice(40)),
    });
  }
  const m = /Records: (\d+)\s+Capacity: (\d+)/.exec(o);
  return { records, size: m ? +m[1] : 0, capacity: m ? +m[2] : 4 };
}

async function exec(lines) {
  if (!child) await start();
  let tr = lines.length ? "Choice: " : "",
    last = "";
  for (const l of lines) {
    if (!child) break;
    tr += l + "\n";
    last = await send(l);
    tr += last; // transcript = prompts + the input typed + program output
  }
  if (!child) await start();
  const stuck = lines.length > 0 && !last.endsWith("Choice: ");
  const msg =
    cut(last)
      .split("\n")
      .map((s) => s.trim())
      .filter(Boolean)
      .pop() || "";
  let state = null;
  if (!stuck) state = parse(cut(await send("6"))); // fresh snapshot of the array after every command
  return { transcript: u(cut(tr)), msg: u(msg), stuck, state };
}

let q = Promise.resolve(); // one command at a time
const enqueue = (f) => {
  const p = q.then(f);
  q = p.catch(() => {});
  return p;
};

http
  .createServer((req, res) => {
    if (req.method === "GET") {
      fs.readFile(path.join(DIR, "index.html"), (e, d) => {
        res.writeHead(e ? 404 : 200, {
          "Content-Type": "text/html; charset=utf-8",
        });
        res.end(e ? "index.html not found next to server.js" : d);
      });
      return;
    }
    let body = "";
    req.on("data", (c) => (body += c));
    req.on("end", async () => {
      const reply = (code, o) => {
        res.writeHead(code, { "Content-Type": "application/json" });
        res.end(JSON.stringify(o));
      };
      try {
        const j = body ? JSON.parse(body) : {};
        if (req.url === "/api/reset")
          return reply(
            200,
            await enqueue(async () => {
              if (child) {
                const c = child;
                child = null;
                c.kill();
              }
              return exec([]);
            }),
          );
        const lines = (Array.isArray(j.lines) ? j.lines : []).map((l) =>
          String(l).replace(/[\r\n]/g, " "),
        );
        reply(200, await enqueue(() => exec(lines)));
      } catch (e) {
        reply(500, { error: String(e.message || e) });
      }
    });
  })
  .listen(PORT, "127.0.0.1", () =>
    console.log(`Open http://localhost:${PORT}`),
  );
process.on("exit", () => {
  if (child) child.kill();
});
