use lotus_engine::{LotusEngine, Method};

fn main() {
    let mut engine = LotusEngine::new();
    engine.set_method(Method::Telex);

    println!("Typing 'a'...");
    let res1 = engine.process_key('a' as u32, false, false);
    println!("Action: {}, Backspace: {}, Chars: {:?}", res1.action, res1.backspace, res1.chars);

    println!("Typing 's'...");
    let res2 = engine.process_key('s' as u32, false, false);
    println!("Action: {}, Backspace: {}, Chars: {:?}", res2.action, res2.backspace, res2.chars);
}
