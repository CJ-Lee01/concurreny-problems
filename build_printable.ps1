param(
    [string]$OutputName = "ConcurrencyProblems",
    [switch]$NoPdf
)

$ErrorActionPreference = "Stop"

$RepoRoot = $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Get-Location).Path
}

$MarkdownPath = Join-Path $RepoRoot "Printable.md"
$HtmlPath = Join-Path $RepoRoot "Printable.html"
$PdfPath = Join-Path $RepoRoot "$OutputName.pdf"

$Problems = @(
    @{ Name = "Barrier"; Dir = "barrier" },
    @{ Name = "Producer Consumer"; Dir = "producer_consumer" },
    @{ Name = "Reader Writer"; Dir = "reader_writer" },
    @{ Name = "Dining Philosophers"; Dir = "dining_philosophers" },
    @{ Name = "Barbershop"; Dir = "barbershop" },
    @{ Name = "FIFO Semaphore"; Dir = "fifo_semaphore" },
    @{ Name = "H2O"; Dir = "h2o" },
    @{ Name = "Cigarette Smokers"; Dir = "cigarette_smokers" }
)

$Languages = @(
    @{ Name = "Blocking C++20"; Dir = "blocking_cpp"; File = "main.cpp"; Fence = "cpp" },
    @{ Name = "Nonblocking C++20"; Dir = "nonblocking_cpp"; File = "main.cpp"; Fence = "cpp" },
    @{ Name = "Go"; Dir = "go"; File = "main.go"; Fence = "go" },
    @{ Name = "Rust"; Dir = "rust"; File = "main.rs"; Fence = "rust" }
)

function Escape-Html {
    param([string]$Value)
    return [System.Net.WebUtility]::HtmlEncode($Value)
}

function Convert-MarkdownToHtml {
    param([string]$Path)

    $Lines = Get-Content -LiteralPath $Path
    $Builder = [System.Text.StringBuilder]::new()
    $InCode = $false
    $InList = $false
    $Paragraph = [System.Collections.Generic.List[string]]::new()

    function Flush-Paragraph {
        if ($Paragraph.Count -gt 0) {
            $Text = ($Paragraph -join " ").Trim()
            [void]$Builder.AppendLine("<p>$(Escape-Html $Text)</p>")
            $Paragraph.Clear()
        }
    }

    function Close-List {
        if ($InList) {
            [void]$Builder.AppendLine("</ul>")
            Set-Variable -Name InList -Value $false -Scope 1
        }
    }

    $Index = 0
    while ($Index -lt $Lines.Count) {
        $Line = $Lines[$Index]

        if ($Line -match '^\s*```') {
            Flush-Paragraph
            Close-List
            if ($InCode) {
                [void]$Builder.AppendLine("</code></pre>")
                $InCode = $false
            } else {
                [void]$Builder.AppendLine("<pre><code>")
                $InCode = $true
            }
            $Index++
            continue
        }

        if ($InCode) {
            [void]$Builder.AppendLine((Escape-Html $Line))
            $Index++
            continue
        }

        if ([string]::IsNullOrWhiteSpace($Line)) {
            Flush-Paragraph
            Close-List
            $Index++
            continue
        }

        if ($Line -match '^(#{1,6})\s+(.+)$') {
            Flush-Paragraph
            Close-List
            $Level = $Matches[1].Length
            $Text = Escape-Html $Matches[2].Trim()
            [void]$Builder.AppendLine("<h$Level>$Text</h$Level>")
            $Index++
            continue
        }

        if ($Line -match '^\s*[-*]\s+(.+)$') {
            Flush-Paragraph
            if (-not $InList) {
                [void]$Builder.AppendLine("<ul>")
                $InList = $true
            }
            [void]$Builder.AppendLine("<li>$(Escape-Html $Matches[1].Trim())</li>")
            $Index++
            continue
        }

        if ($Line.Trim().StartsWith("|") -and $Line.Trim().EndsWith("|")) {
            Flush-Paragraph
            Close-List

            $TableRows = [System.Collections.Generic.List[string]]::new()
            while ($Index -lt $Lines.Count -and $Lines[$Index].Trim().StartsWith("|") -and $Lines[$Index].Trim().EndsWith("|")) {
                if ($Lines[$Index] -notmatch '^\s*\|(\s*:?-+:?\s*\|)+\s*$') {
                    $TableRows.Add($Lines[$Index])
                }
                $Index++
            }

            [void]$Builder.AppendLine("<table>")
            for ($RowIndex = 0; $RowIndex -lt $TableRows.Count; $RowIndex++) {
                $Cells = $TableRows[$RowIndex].Trim().Trim("|").Split("|") | ForEach-Object { Escape-Html $_.Trim() }
                $CellTag = if ($RowIndex -eq 0) { "th" } else { "td" }
                [void]$Builder.AppendLine("<tr>")
                foreach ($Cell in $Cells) {
                    [void]$Builder.AppendLine("<$CellTag>$Cell</$CellTag>")
                }
                [void]$Builder.AppendLine("</tr>")
            }
            [void]$Builder.AppendLine("</table>")
            continue
        }

        $Paragraph.Add($Line.Trim())
        $Index++
    }

    Flush-Paragraph
    Close-List

    if ($InCode) {
        [void]$Builder.AppendLine("</code></pre>")
    }

    return $Builder.ToString()
}

function Add-MarkdownDocument {
    param(
        [System.Text.StringBuilder]$Builder,
        [string]$Path
    )

    if (Test-Path -LiteralPath $Path) {
        [void]$Builder.AppendLine((Get-Content -LiteralPath $Path -Raw).TrimEnd())
        [void]$Builder.AppendLine()
    }
}

function Get-BrowserPath {
    $Candidates = @()

    foreach ($CommandName in @("chrome", "msedge")) {
        $Command = Get-Command $CommandName -ErrorAction SilentlyContinue
        if ($Command) {
            $Candidates += $Command.Source
        }
    }

    $Candidates += @(
        (Join-Path $env:ProgramFiles "Google\Chrome\Application\chrome.exe"),
        (Join-Path ${env:ProgramFiles(x86)} "Google\Chrome\Application\chrome.exe"),
        (Join-Path $env:ProgramFiles "Microsoft\Edge\Application\msedge.exe"),
        (Join-Path ${env:ProgramFiles(x86)} "Microsoft\Edge\Application\msedge.exe")
    )

    foreach ($Candidate in $Candidates | Select-Object -Unique) {
        if ($Candidate -and (Test-Path -LiteralPath $Candidate)) {
            return $Candidate
        }
    }

    return $null
}

$Markdown = [System.Text.StringBuilder]::new()
[void]$Markdown.AppendLine("# Classical Concurrency Problems - Printable Bundle")
[void]$Markdown.AppendLine()

foreach ($DocName in @("Problems.md", "Specification.MD", "README.md")) {
    Add-MarkdownDocument $Markdown (Join-Path $RepoRoot $DocName)
}

[void]$Markdown.AppendLine("# Source Code")
[void]$Markdown.AppendLine()

foreach ($Problem in $Problems) {
    [void]$Markdown.AppendLine("## $($Problem.Name)")
    [void]$Markdown.AppendLine()

    foreach ($Language in $Languages) {
        $SourcePath = Join-Path $RepoRoot (Join-Path $Problem.Dir (Join-Path $Language.Dir $Language.File))
        if (-not (Test-Path -LiteralPath $SourcePath)) {
            continue
        }

        $RelativePath = Join-Path $Problem.Dir (Join-Path $Language.Dir $Language.File)
        [void]$Markdown.AppendLine("### $($Language.Name) - ``$RelativePath``")
        [void]$Markdown.AppendLine()
        [void]$Markdown.AppendLine("````$($Language.Fence)")
        [void]$Markdown.AppendLine((Get-Content -LiteralPath $SourcePath -Raw).TrimEnd())
        [void]$Markdown.AppendLine("````")
        [void]$Markdown.AppendLine()
    }
}

Set-Content -LiteralPath $MarkdownPath -Value $Markdown.ToString() -Encoding UTF8

$Html = [System.Text.StringBuilder]::new()
[void]$Html.AppendLine("<!doctype html>")
[void]$Html.AppendLine("<html lang=""en"">")
[void]$Html.AppendLine("<head>")
[void]$Html.AppendLine("<meta charset=""utf-8"">")
[void]$Html.AppendLine("<title>Classical Concurrency Problems</title>")
[void]$Html.AppendLine("<style>")
[void]$Html.AppendLine("@page { size: A4; margin: 18mm 14mm; }")
[void]$Html.AppendLine("body { font-family: ""Segoe UI"", Arial, sans-serif; color: #161616; line-height: 1.45; max-width: 980px; margin: 0 auto; padding: 28px; }")
[void]$Html.AppendLine("h1 { font-size: 28px; border-bottom: 2px solid #222; padding-bottom: 8px; margin: 0 0 18px; }")
[void]$Html.AppendLine("h2 { font-size: 22px; margin: 30px 0 10px; break-after: avoid; }")
[void]$Html.AppendLine("h3 { font-size: 16px; margin: 22px 0 8px; break-after: avoid; }")
[void]$Html.AppendLine("pre { background: #f6f7f8; border: 1px solid #d8dde3; border-radius: 6px; padding: 12px; overflow-wrap: anywhere; white-space: pre-wrap; font: 13px/1.4 Consolas, ""Courier New"", monospace; break-inside: avoid; }")
[void]$Html.AppendLine("code { font-family: Consolas, ""Courier New"", monospace; }")
[void]$Html.AppendLine(".toc { background: #f6f7f8; border: 1px solid #d8dde3; border-radius: 6px; padding: 12px 16px; margin-bottom: 24px; }")
[void]$Html.AppendLine(".toc a { color: #064f8f; text-decoration: none; }")
[void]$Html.AppendLine(".doc-block { background: #fff; border-left: 4px solid #8091a5; padding: 8px 12px; margin-bottom: 14px; }")
[void]$Html.AppendLine(".doc-block table { width: 100%; border-collapse: collapse; margin: 12px 0; }")
[void]$Html.AppendLine(".doc-block th, .doc-block td { border: 1px solid #d8dde3; padding: 6px 8px; text-align: left; vertical-align: top; }")
[void]$Html.AppendLine(".doc-block th { background: #eef1f4; }")
[void]$Html.AppendLine(".doc-block ul { margin: 8px 0 12px 24px; padding: 0; }")
[void]$Html.AppendLine(".doc-block p { margin: 0 0 10px; }")
[void]$Html.AppendLine(".path { color: #52616f; font-weight: 400; }")
[void]$Html.AppendLine(".page-break { break-before: page; }")
[void]$Html.AppendLine("@media print { body { max-width: none; padding: 0; } pre { font-size: 12px; } a { color: inherit; } }")
[void]$Html.AppendLine("</style>")
[void]$Html.AppendLine("</head>")
[void]$Html.AppendLine("<body>")
[void]$Html.AppendLine("<h1>Classical Concurrency Problems</h1>")
[void]$Html.AppendLine("<div class=""toc""><strong>Contents</strong><ol>")

foreach ($Problem in $Problems) {
    [void]$Html.AppendLine("<li><a href=""#$($Problem.Dir)"">$($Problem.Name)</a></li>")
}

[void]$Html.AppendLine("</ol></div>")

foreach ($DocName in @("Problems.md", "Specification.MD", "README.md")) {
    $DocPath = Join-Path $RepoRoot $DocName
    if (Test-Path -LiteralPath $DocPath) {
        [void]$Html.AppendLine("<h2>$DocName</h2>")
        [void]$Html.AppendLine("<div class=""doc-block"">")
        [void]$Html.AppendLine((Convert-MarkdownToHtml $DocPath))
        [void]$Html.AppendLine("</div>")
    }
}

[void]$Html.AppendLine("<h2 class=""page-break"">Source Code</h2>")

foreach ($Problem in $Problems) {
    [void]$Html.AppendLine("<h2 id=""$($Problem.Dir)"">$($Problem.Name)</h2>")

    foreach ($Language in $Languages) {
        $SourcePath = Join-Path $RepoRoot (Join-Path $Problem.Dir (Join-Path $Language.Dir $Language.File))
        if (-not (Test-Path -LiteralPath $SourcePath)) {
            continue
        }

        $RelativePath = Join-Path $Problem.Dir (Join-Path $Language.Dir $Language.File)
        [void]$Html.AppendLine("<h3>$($Language.Name) <span class=""path"">$RelativePath</span></h3>")
        [void]$Html.AppendLine("<pre><code>")
        [void]$Html.AppendLine((Escape-Html (Get-Content -LiteralPath $SourcePath -Raw)))
        [void]$Html.AppendLine("</code></pre>")
    }
}

[void]$Html.AppendLine("</body></html>")
Set-Content -LiteralPath $HtmlPath -Value $Html.ToString() -Encoding UTF8

Write-Host "Wrote $MarkdownPath"
Write-Host "Wrote $HtmlPath"

if ($NoPdf) {
    Write-Host "Skipped PDF export because -NoPdf was provided."
    exit 0
}

$BrowserPath = Get-BrowserPath
if (-not $BrowserPath) {
    Write-Warning "Could not find Chrome or Edge. Open Printable.html in a browser and print it to PDF."
    exit 0
}

$TempPdfPath = Join-Path $RepoRoot "$OutputName.$([Guid]::NewGuid().ToString("N")).tmp.pdf"
$HtmlUri = [System.Uri]::new($HtmlPath).AbsoluteUri
$BrowserArgs = @(
    "--headless",
    "--disable-gpu",
    "--disable-crash-reporter",
    "--print-to-pdf=$TempPdfPath",
    $HtmlUri
)

& $BrowserPath @BrowserArgs | Out-Host

if (Test-Path -LiteralPath $TempPdfPath) {
    Move-Item -LiteralPath $TempPdfPath -Destination $PdfPath -Force
    Write-Host "Wrote $PdfPath"
} else {
    throw "PDF export failed. Open Printable.html in a browser and print it to PDF."
}
