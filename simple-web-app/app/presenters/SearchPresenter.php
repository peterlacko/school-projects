<?php

use Nette\Application\UI\Form;

class SearchPresenter extends BasePresenter
{
    private $skolaRepository;
    private $fakultaRepository;
    private $oborRepository;
    private $akceRepository;
    private $predmetRepository;
    private $oblibeneRepository;
    private $dodRepository;
    private $resultSkola;
    private $resultFakulta;
    private $resultObor;
    private $resultPredmet;
    private $resultAkce;
    private $resultDod;
    private $flags = array();

    protected function startup()
    {
	parent::startup();
	$this->skolaRepository = $this->context->skolaRepository;
	$this->fakultaRepository = $this->context->fakultaRepository;
	$this->oborRepository = $this->context->oborRepository;
	$this->akceRepository = $this->context->akceRepository;
	$this->predmetRepository = $this->context->predmetRepository;
	$this->oblibeneRepository = $this->context->oblibeneRepository;
	$this->dodRepository = $this->context->dodRepository;
    }

    public function actionResults($phrase, $skola, $fakulta, $obor, $predmet, $akce, $dod)
    {
	if ($skola) {
	    $result = $this->skolaRepository->search($phrase);
	    count($result) == 0 ? : $this->resultSkola = $result;
	}
	$this->flags['skola'] = $skola;

	if ($fakulta) {
	    $result = $this->fakultaRepository->search($phrase);
	    count($result) == 0? : $this->resultFakulta = $result;
	}
	$this->flags['fakulta'] = $fakulta;

	if ($obor) {
	    $obory2 = array();
	    $obory = $this->oborRepository->search($phrase);
	    foreach ($obory as $obor) {
		$obor['Nazev_fakulty'] = $this->fakultaRepository
		    ->findBy(array('ID_fakulty' => $obor->ID_fakulty))->fetch()->Nazev_fakulty;
		$obory2[] = $obor;
	    }
	    empty($obory2) ? : $this->resultObor = $obory2;
	}
	$this->flags['obor'] = $obor;

	if ($predmet) {
	    $predmety2 = array();
	    $predmety = $this->predmetRepository->search($phrase);
	    foreach ($predmety as $predmet) {
		$predmet['Nazev_fakulty'] = $this->fakultaRepository
		    ->findBy(array('ID_fakulty' => $predmet->ID_fakulty))->fetch()->Nazev_fakulty;
		$predmety2[] = $predmet;
	    }
	    empty($predmety2) ? : $this->resultPredmet = $predmety2;
	}
	$this->flags['predmet'] = $predmet;

	if ($akce) {
	    $akce2 = array();
	    $akce = $this->akceRepository->search($phrase);
	    foreach ($akce as $akca) {
		$akca['Nazev_fakulty'] = $this->fakultaRepository
		    ->findBy(array('ID_fakulty' => $akca->ID_fakulty))->fetch()->Nazev_fakulty;
		$akce2[] = $akca;
	    }
	    empty($akce2) ? : $this->resultAkce = $akce2;
	}
	$this->flags['akce'] = $akce;

	if ($dod) {
	    $dod2 = array();
	    $dods = $this->dodRepository->search($phrase);
	    foreach ($dods as $dod) {
		$dod['Nazev_fakulty'] = $this->fakultaRepository
		    ->findBy(array('ID_fakulty' => $dod->ID_fakulty))->fetch()->Nazev_fakulty;
		$dod2[] = $dod;
	    }
	    empty($dod2) ? : $this->resultDod = $dod2;
	}
	$this->flags['dod'] = $dod;

    }
    public function renderResults()
    {
	$this->template->skoly = $this->resultSkola;
	$this->template->fakulty = $this->resultFakulta;
	$this->template->obory = $this->resultObor;
	$this->template->predmety = $this->resultPredmet;
	$this->template->akce = $this->resultAkce;
	$this->template->dods = $this->resultDod;
	$this->template->flags = $this->flags;
    }

    protected function createComponentSearchForm()
    {
	$form = new Form;
	$form->addText('search_phrase', 'Zadejte hledaný výraz:', 30)
		->setRequired('Zadejte hledaný výraz')
	    ->addRule(Form::MIN_LENGTH, 'Hledaný výraz musí mít alespoň 3 znaky', 3);
	$form->addCheckbox('skola', 'Škola')
	    ->setDefaultValue(TRUE);
	$form->addCheckbox('fakulta', 'Fakulta')
	    ->setDefaultValue(TRUE);
	$form->addCheckbox('obor', 'Obor')
	    ->setDefaultValue(TRUE);
	$form->addCheckbox('predmet', 'Předmět')
	    ->setDefaultValue(TRUE);
	$form->addCheckbox('akce', 'Akce')
	    ->setDefaultValue(TRUE);
	$form->addCheckbox('dod', 'DOD')
	    ->setDefaultValue(TRUE);
	$form->addSubmit('search', 'Hledat')
	    ->onClick[] = $this->searchFormSubmitted;
	$form->addSubmit('cancel', 'Vymazat formulář')
	    ->setValidationScope(FALSE)
	    ->onClick[] = $this->searchFormCancelled;
	return $form;
    }

    public function searchFormSubmitted(Nette\Forms\Controls\SubmitButton $btn)
    {
	$values = $btn->form->getValues();
	$this->redirect('Search:results', array(
	    'phrase'=>$values->search_phrase,
	    'skola'=>$values->skola,
	    'fakulta'=>$values->fakulta,
	    'obor'=>$values->obor,
	    'predmet' => $values->predmet,
	    'akce'=>$values->akce,
	    'dod'=>$values->dod
	));
    }

    public function searchFormCancelled(Nette\Forms\Controls\SubmitButton $btn)
    {
	$btn->form->setValues(array(), TRUE);
    }
}
