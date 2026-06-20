import os
from reportlab.lib.pagesizes import letter
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Preformatted, HRFlowable
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.enums import TA_CENTER, TA_LEFT
from reportlab.lib import colors

pdf_path = "LRU_Cache_Design_Document.pdf"
doc = SimpleDocTemplate(pdf_path, pagesize=letter,
                        rightMargin=54, leftMargin=54,
                        topMargin=54, bottomMargin=54)

styles = getSampleStyleSheet()

# Define Custom Styles to match the sleek document layout
title_style = ParagraphStyle(
    'DocTitle',
    parent=styles['Heading1'],
    fontSize=24,
    leading=28,
    textColor=colors.HexColor("#1A365D"),
    alignment=TA_LEFT,
    spaceAfter=15
)

h1_style = ParagraphStyle(
    'SectionH1',
    parent=styles['Heading2'],
    fontSize=16,
    leading=20,
    textColor=colors.HexColor("#2B6CB0"),
    spaceBefore=14,
    spaceAfter=6,
    keepWithNext=True
)

h2_style = ParagraphStyle(
    'SectionH2',
    parent=styles['Heading3'],
    fontSize=12,
    leading=16,
    textColor=colors.HexColor("#4A5568"),
    spaceBefore=10,
    spaceAfter=4,
    keepWithNext=True
)

body_style = ParagraphStyle(
    'DocBody',
    parent=styles['BodyText'],
    fontSize=10,
    leading=14,
    textColor=colors.HexColor("#2D3748"),
    spaceAfter=6
)

bullet_style = ParagraphStyle(
    'DocBullet',
    parent=styles['Normal'],
    fontSize=10,
    leading=14,
    textColor=colors.HexColor("#2D3748"),
    leftIndent=15,
    firstLineIndent=-10,
    spaceAfter=4
)

code_style = ParagraphStyle(
    'DocCode',
    fontName='Courier',
    fontSize=8.5,
    leading=11,
    textColor=colors.HexColor("#1A202C"),
    backColor=colors.HexColor("#EDF2F7"),
    borderColor=colors.HexColor("#CBD5E0"),
    borderWidth=0.5,
    borderPadding=8,
    spaceBefore=8,
    spaceAfter=8
)

story = []

# --- Document Content ---
story.append(Paragraph("LRU File Block Cache: Design & Implementation", title_style))
story.append(HRFlowable(width="100%", thickness=2, color=colors.HexColor("#2B6CB0"), spaceAfter=15))

story.append(Paragraph("🎯 The Goal", h1_style))
story.append(Paragraph("Design and implement a simple Least Recently Used (LRU) caching mechanism for file blocks as a response to a personal coding challenge.", body_style))

story.append(Paragraph("⚠️ Disclaimers", h1_style))
story.append(Paragraph("• <b>Human-Written:</b> This document is hand-written. For an AI-assisted version with automated spell-checks, see <i>README-AI.md</i>.", bullet_style))
story.append(Paragraph("• <b>Licensing:</b> See <i>LICENSE.md</i> for full terms.", bullet_style))
story.append(Paragraph("• <b>Credits:</b> Original work inspired by existing open-source software. Individual subsystem licenses apply.", bullet_style))
story.append(Paragraph("• <b>Warranty:</b> Provided \"as-is\" without any explicit or implied guarantees. Use at your own risk.", bullet_style))
story.append(Paragraph("• <b>Alternative Solutions:</b> This is a conceptual demonstration. Conduct your own Cost-Benefit Analysis (CBA) before production use.", bullet_style))

story.append(Paragraph("📌 Sub-Goals", h1_style))
story.append(Paragraph("• Keep it light-weight and simple.", bullet_style))
story.append(Paragraph("• Keep it as generic as possible.", bullet_style))
story.append(Paragraph("• Make it easy for interns or students to read.", bullet_style))

story.append(Paragraph("📖 Context", h1_style))
story.append(Paragraph("Filesystems and document databases operate at the file level. Applications can achieve massive performance gains by caching specific file blocks during active I/O sessions.", body_style))

story.append(Paragraph("⚙️ Technical Considerations", h1_style))
story.append(Paragraph("Cache Tagging", h2_style))
story.append(Paragraph("• <b>Global Context:</b> Combine File Block Number (FBN) with the Inode number to persist cache tags across sessions.", bullet_style))
story.append(Paragraph("• <b>Transient Context:</b> Combine FBN with the File Descriptor (FD) for temporary, session-based caching.", bullet_style))
story.append(Paragraph("• <b>LBA Alternative:</b> Use the Logical Block Address (LBA), which requires a unique volume identifier (like a UUID) to prevent conflicts.", bullet_style))

story.append(Paragraph("Fetching vs. Deriving FBN", h2_style))
story.append(Paragraph("• <b>Fetching:</b> Traverses inodes directly. This scales well but adds high complexity, so it is skipped for this project.", bullet_style))
story.append(Paragraph("• <b>Deriving:</b> Calculated mathematically from the file offset: <code>(offset / FILE_BLOCK_SIZE)</code>.", bullet_style))

story.append(Paragraph("File Block Size Dynamics", h2_style))
story.append(Paragraph("• Typical personal computing block sizes range from 1KB to 4KB (usually default to 4KB).", bullet_style))
story.append(Paragraph("• Large file servers may scale up to 1MB+ blocks.", bullet_style))
story.append(Paragraph("• Uniformity can be enforced by setting <code>FILE_BLOCK_SIZE</code> manually.", bullet_style))

story.append(Paragraph("Out of Scope", h2_style))
story.append(Paragraph("• <b>Custom Application Tags:</b> Internal UUIDs or application-specific references are skipped.", bullet_style))
story.append(Paragraph("• <b>Sparse Files:</b> Empty blocks in sparse files can waste cache lines; handling this is left as a future enhancement.", bullet_style))
story.append(Paragraph("• <b>Orphan Control:</b> Automated scanning and clearing of orphaned blocks is excluded.", bullet_style))
story.append(Paragraph("• <b>Session Management:</b> Saving and restoring cache sessions is excluded.", bullet_style))

story.append(Paragraph("🏗️ Design Decisions", h1_style))
story.append(Paragraph("Filesystem caching is complex. This implementation intentionally prioritises conceptual clarity over production-grade edge-case handling.", body_style))

story.append(Paragraph("1. Cache Line Look-Up", h2_style))
story.append(Paragraph("<b>Doubly-Linked Lists:</b> Simple to implement and test, but has linear time complexity O(n). It destroys CPU cache lines and triggers random RAM reads at scale. Unsuitable for more than a few hundred blocks.", body_style))
story.append(Paragraph("<b>Hash Tables:</b> Ideal constant time complexity O(1). However, it requires extremely high memory consumption (e.g., a 32-bit hash table can require up to 16GB of RAM).", body_style))
story.append(Paragraph("<b>Binary Trees (Selected):</b> Highly balanced performance using self-balancing structures like Red-Black Trees. Logarithmic time complexity O(log n). Uses ready-made standard library structures (<code>std::set</code> or <code>std::map</code>).", body_style))

story.append(Paragraph("2. LRU Chain Management", h2_style))
story.append(Paragraph("To manage expiration tracking, we selected a <b>hybrid approach</b>. Real-world systems use an array/vector for recent hits, synchronising the main LRU tree periodically to bypass heavy tree rebalancing on every read operation.", body_style))

story.append(Paragraph("🔒 Locks and Race Conditions", h1_style))
story.append(Paragraph("Concurrency is a deep problem space. This design requires strict locking mechanisms (or lock-free algorithms) across both the trees and linked lists to maintain thread safety and data consistency.", body_style))

doc.build(story)
print("PDF Generated successfully")
print(f"File size: {os.path.getsize(pdf_path)} bytes")

