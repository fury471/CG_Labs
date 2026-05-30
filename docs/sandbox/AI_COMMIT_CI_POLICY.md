# AI Commit CI Policy

This rule applies to AI-assisted online edits on milestone branches.

## Rule

AI-created intermediate commits must include:

```text
[skip ci]
```

Only pull request creation and pull request validation should run CI.

## Required behavior

During implementation:

```text
scene: add OBJ mesh loader [skip ci]
gfx: add mesh upload path [skip ci]
app: add mesh visibility toggle [skip ci]
```

When the milestone is locally validated and the pull request is opened, do not use CI-skip wording in the pull request title or description.

## Reason

Intermediate AI commits can be frequent. Skipping CI for those commits avoids unnecessary GitHub Actions runs. CI should run once the milestone is ready for review through the pull request workflow.
