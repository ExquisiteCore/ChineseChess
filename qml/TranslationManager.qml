import QtQuick

QtObject {
    id: translationManager

    // 当前语言
    property string currentLanguage: "zh_CN"  // zh_CN 或 en_US

    // 翻译字典
    property var translations: ({
        "zh_CN": {
            // 菜单页面
            "game_title": "中国象棋",
            "single_player": "单人对战",
            "two_player": "双人对战",
            "settings": "设置",
            "exit": "退出游戏",

            // 游戏页面
            "back_to_menu": "返回菜单",
            "current_turn": "当前回合",
            "red_turn": "红方",
            "black_turn": "黑方",
            "ai_thinking": "AI思考中",
            "undo": "悔棋",
            "redo": "重做",
            "restart": "重新开始",
            "game_info": "游戏信息",
            "game_status": "游戏状态",
            "status": "状态",
            "move_count": "步数",
            "total_moves": "总步数",
            "ai_control": "AI控制",
            "ai_opponent": "AI对手",
            "difficulty": "难度",
            "enable_ai": "启用AI",
            "enabled": "已启用",
            "disabled": "已禁用",
            "move_history": "走子历史",
            "operations": "操作",
            "hint": "提示",
            "offer_draw": "求和",
            "resign": "认输",
            "save_game": "保存局面",
            "load_game": "加载局面",
            "play_again": "再来一局",

            // AI难度
            "easy": "简单",
            "medium": "中等",
            "hard": "困难",
            "expert": "专家",

            // 游戏状态
            "red_win_checkmate": "红方胜 - 黑方被将死",
            "black_win_checkmate": "黑方胜 - 红方被将死",
            "red_win_king": "红方胜 - 黑方将被吃",
            "black_win_king": "黑方胜 - 红方帅被吃",
            "draw_stalemate": "和棋 - 困毙",
            "in_check": "被将军！",
            "red_to_move": "红方走棋",
            "black_to_move": "黑方走棋",

            // 对话框
            "save_game_title": "保存局面",
            "save_game_desc": "复制下面的FEN字符串保存当前局面：",
            "copy": "复制",
            "close": "关闭",
            "load_game_title": "加载局面",
            "load_game_desc": "粘贴FEN字符串加载局面：",
            "fen_error": "FEN 字符串格式错误，请检查后重试",
            "load": "加载",
            "game_over": "游戏结束",
            "victory": "胜利！",
            "defeat": "失败",
            "draw_offer": "求和通知",
            "draw_requested": "对方提出求和",

            // 设置页面
            "settings_title": "设置",
            "sound_volume": "音效音量",
            "language": "语言",
            "chinese": "简体中文",
            "english": "English",
            "apply": "应用",
            "cancel": "取消",
            "sound_enabled": "启用音效"
        },
        "en_US": {
            // Menu Page
            "game_title": "Chinese Chess",
            "single_player": "Single Player",
            "two_player": "Two Players",
            "settings": "Settings",
            "exit": "Exit Game",

            // Game Page
            "back_to_menu": "Back to Menu",
            "current_turn": "Current Turn",
            "red_turn": "Red",
            "black_turn": "Black",
            "ai_thinking": "AI Thinking",
            "undo": "Undo",
            "redo": "Redo",
            "restart": "Restart",
            "game_info": "Game Info",
            "game_status": "Game Status",
            "status": "Status",
            "move_count": "Moves",
            "total_moves": "Total Moves",
            "ai_control": "AI Control",
            "ai_opponent": "AI Opponent",
            "difficulty": "Difficulty",
            "enable_ai": "Enable AI",
            "enabled": "Enabled",
            "disabled": "Disabled",
            "move_history": "Move History",
            "operations": "Operations",
            "hint": "Hint",
            "offer_draw": "Offer Draw",
            "resign": "Resign",
            "save_game": "Save Game",
            "load_game": "Load Game",
            "play_again": "Play Again",

            // AI Difficulty
            "easy": "Easy",
            "medium": "Medium",
            "hard": "Hard",
            "expert": "Expert",

            // Game Status
            "red_win_checkmate": "Red Wins - Black Checkmated",
            "black_win_checkmate": "Black Wins - Red Checkmated",
            "red_win_king": "Red Wins - Black King Captured",
            "black_win_king": "Black Wins - Red King Captured",
            "draw_stalemate": "Draw - Stalemate",
            "in_check": "In Check!",
            "red_to_move": "Red to Move",
            "black_to_move": "Black to Move",

            // Dialogs
            "save_game_title": "Save Game",
            "save_game_desc": "Copy the FEN string below to save current position:",
            "copy": "Copy",
            "close": "Close",
            "load_game_title": "Load Game",
            "load_game_desc": "Paste FEN string to load position:",
            "fen_error": "Invalid FEN string, please check and try again",
            "load": "Load",
            "game_over": "Game Over",
            "victory": "Victory!",
            "defeat": "Defeat",
            "draw_offer": "Draw Offer",
            "draw_requested": "Opponent offers draw",

            // Settings Page
            "settings_title": "Settings",
            "sound_volume": "Sound Volume",
            "language": "Language",
            "chinese": "简体中文",
            "english": "English",
            "apply": "Apply",
            "cancel": "Cancel",
            "sound_enabled": "Enable Sound"
        }
    })

    // 获取翻译文本的函数
    function tr(key) {
        var lang = translations[currentLanguage]
        if (lang && lang[key]) {
            return lang[key]
        }
        // 如果找不到翻译，返回key本身
        return key
    }

    // 切换语言
    function setLanguage(lang) {
        if (translations[lang]) {
            currentLanguage = lang
            console.log("语言切换到:", lang)
        }
    }
}
