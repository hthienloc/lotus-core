// ==UserScript==
// @name         Jules Session & Message Copier
// @namespace    http://tampermonkey.net/
// @version      1.7
// @description  Copy Jules message with Session URL (Agent Only & CSP Compliant)
// @author       Gemini Orchestrator
// @match        https://jules.google.com/session/*
// @icon         https://www.google.com/s2/favicons?sz=64&domain=google.com
// @grant        none
// ==/UserScript==

(function() {
    'use strict';

    console.log('%c[Lotus Helper] Script V1.7 Loaded (Agent Only)', 'color: #34a853; font-weight: bold;');

    function copyToClipboard(text) {
        const textArea = document.createElement("textarea");
        textArea.value = text;
        document.body.appendChild(textArea);
        textArea.select();
        try {
            document.execCommand('copy');
            console.log('[Lotus Helper] Copied successfully');
        } catch (err) {
            console.error('[Lotus Helper] Copy failed', err);
        }
        document.body.removeChild(textArea);
    }

    function createSafeButton(textGetter) {
        const btn = document.createElement('button');
        btn.textContent = '📋 COPY JULES MESSAGE + SESSION';
        
        const s = btn.style;
        s.display = 'block';
        s.marginTop = '12px';
        s.marginBottom = '12px';
        s.padding = '8px 16px';
        s.backgroundColor = '#1a73e8';
        s.color = '#ffffff';
        s.border = 'none';
        s.borderRadius = '8px';
        s.fontSize = '12px';
        s.fontWeight = 'bold';
        s.cursor = 'pointer';
        s.boxShadow = '0 2px 5px rgba(0,0,0,0.2)';
        s.zIndex = '9999';

        btn.onclick = (e) => {
            e.preventDefault();
            e.stopPropagation();
            const sessionUrl = window.location.href;
            const message = textGetter();
            copyToClipboard(`Session URL: ${sessionUrl}\n\nJules Message:\n${message}`);
            
            const originalText = btn.textContent;
            btn.textContent = '✅ COPIED!';
            btn.style.backgroundColor = '#34a853';
            setTimeout(() => {
                btn.textContent = originalText;
                btn.style.backgroundColor = '#1a73e8';
            }, 2000);
        };

        return btn;
    }

    function scanAndInject() {
        // CHỈ tìm các message container của Jules (Agent)
        const containers = document.querySelectorAll('swebot-agent-chat-bubble .message-container');
        
        containers.forEach(container => {
            if (container.getAttribute('data-gemini-injected') === 'true') return;
            
            // Tìm vùng hiển thị Markdown
            const content = container.querySelector('swebot-markdown-viewer') || container;
            
            const btn = createSafeButton(() => content.innerText);
            container.appendChild(btn);
            container.setAttribute('data-gemini-injected', 'true');
            console.log('[Lotus Helper] Jules message button injected');
        });
    }

    // Quét liên tục để bắt kịp Angular re-render
    setInterval(scanAndInject, 1000);
    scanAndInject();
})();
